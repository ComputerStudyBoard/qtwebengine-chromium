// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/frame_host/frame_tree.h"

#include <stddef.h>

#include <queue>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/containers/hash_tables.h"
#include "base/lazy_instance.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/browser/frame_host/navigator.h"
#include "content/browser/frame_host/render_frame_host_factory.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/frame_host/render_frame_proxy_host.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/common/input_messages.h"
#include "content/common/site_isolation_policy.h"
#include "third_party/WebKit/public/web/WebSandboxFlags.h"

namespace content {

namespace {

// Used with FrameTree::ForEach() to search for the FrameTreeNode
// corresponding to |frame_tree_node_id| within a specific FrameTree.
bool FrameTreeNodeForId(int frame_tree_node_id,
                        FrameTreeNode** out_node,
                        FrameTreeNode* node) {
  if (node->frame_tree_node_id() == frame_tree_node_id) {
    *out_node = node;
    // Terminate iteration once the node has been found.
    return false;
  }
  return true;
}

// Used with FrameTree::ForEach() to search for the FrameTreeNode with the given
// |name| within a specific FrameTree.
bool FrameTreeNodeForName(const std::string& name,
                          FrameTreeNode** out_node,
                          FrameTreeNode* node) {
  if (node->frame_name() == name) {
    *out_node = node;
    // Terminate iteration once the node has been found.
    return false;
  }
  return true;
}

bool CreateProxyForSiteInstance(const scoped_refptr<SiteInstance>& instance,
                                FrameTreeNode* node) {
  // If a new frame is created in the current SiteInstance, other frames in
  // that SiteInstance don't need a proxy for the new frame.
  SiteInstance* current_instance =
      node->render_manager()->current_frame_host()->GetSiteInstance();
  if (current_instance != instance.get())
    node->render_manager()->CreateRenderFrameProxy(instance.get());
  return true;
}

// Helper function used with FrameTree::ForEach() for retrieving the total
// loading progress and number of frames in a frame tree.
bool CollectLoadProgress(double* progress,
                         int* frame_count,
                         FrameTreeNode* node) {
  // Ignore the current frame if it has not started loading.
  if (!node->has_started_loading())
    return true;

  // Collect progress.
  *progress += node->loading_progress();
  (*frame_count)++;
  return true;
}

// Helper function used with FrameTree::ForEach() to reset the load progress.
bool ResetNodeLoadProgress(FrameTreeNode* node) {
  node->reset_loading_progress();
  return true;
}

// Helper function used with FrameTree::ForEach() to check if at least one of
// the nodes is loading.
bool IsNodeLoading(bool* is_loading, FrameTreeNode* node) {
  if (node->IsLoading()) {
    // There is at least one node loading, so abort traversal.
    *is_loading = true;
    return false;
  }
  return true;
}

// Helper function used with FrameTree::ForEach to collect SiteInstances
// involved in rendering a single FrameTree (which is a subset of SiteInstances
// in main frame's proxy_hosts_ because of openers).
bool CollectSiteInstances(std::set<SiteInstance*>* set, FrameTreeNode* node) {
  set->insert(node->current_frame_host()->GetSiteInstance());
  return true;
}

}  // namespace

FrameTree::FrameTree(Navigator* navigator,
                     RenderFrameHostDelegate* render_frame_delegate,
                     RenderViewHostDelegate* render_view_delegate,
                     RenderWidgetHostDelegate* render_widget_delegate,
                     RenderFrameHostManager::Delegate* manager_delegate)
    : render_frame_delegate_(render_frame_delegate),
      render_view_delegate_(render_view_delegate),
      render_widget_delegate_(render_widget_delegate),
      manager_delegate_(manager_delegate),
      root_(new FrameTreeNode(this,
                              navigator,
                              render_frame_delegate,
                              render_view_delegate,
                              render_widget_delegate,
                              manager_delegate,
                              // The top-level frame must always be in a
                              // document scope.
                              blink::WebTreeScopeType::Document,
                              std::string(),
                              blink::WebSandboxFlags::None,
                              blink::WebFrameOwnerProperties())),
      focused_frame_tree_node_id_(-1),
      load_progress_(0.0) {}

FrameTree::~FrameTree() {
  delete root_;
  root_ = nullptr;
}

FrameTreeNode* FrameTree::FindByID(int frame_tree_node_id) {
  FrameTreeNode* node = nullptr;
  ForEach(base::Bind(&FrameTreeNodeForId, frame_tree_node_id, &node));
  return node;
}

FrameTreeNode* FrameTree::FindByRoutingID(int process_id, int routing_id) {
  RenderFrameHostImpl* render_frame_host =
      RenderFrameHostImpl::FromID(process_id, routing_id);
  if (render_frame_host) {
    FrameTreeNode* result = render_frame_host->frame_tree_node();
    if (this == result->frame_tree())
      return result;
  }

  RenderFrameProxyHost* render_frame_proxy_host =
      RenderFrameProxyHost::FromID(process_id, routing_id);
  if (render_frame_proxy_host) {
    FrameTreeNode* result = render_frame_proxy_host->frame_tree_node();
    if (this == result->frame_tree())
      return result;
  }

  return nullptr;
}

FrameTreeNode* FrameTree::FindByName(const std::string& name) {
  if (name.empty())
    return root_;

  FrameTreeNode* node = nullptr;
  ForEach(base::Bind(&FrameTreeNodeForName, name, &node));
  return node;
}

void FrameTree::ForEach(
    const base::Callback<bool(FrameTreeNode*)>& on_node) const {
  ForEach(on_node, nullptr);
}

void FrameTree::ForEach(
    const base::Callback<bool(FrameTreeNode*)>& on_node,
    FrameTreeNode* skip_this_subtree) const {
  std::queue<FrameTreeNode*> queue;
  queue.push(root_);

  while (!queue.empty()) {
    FrameTreeNode* node = queue.front();
    queue.pop();
    if (skip_this_subtree == node)
      continue;

    if (!on_node.Run(node))
      break;

    for (size_t i = 0; i < node->child_count(); ++i)
      queue.push(node->child_at(i));
  }
}

bool FrameTree::AddFrame(
    FrameTreeNode* parent,
    int process_id,
    int new_routing_id,
    blink::WebTreeScopeType scope,
    const std::string& frame_name,
    blink::WebSandboxFlags sandbox_flags,
    const blink::WebFrameOwnerProperties& frame_owner_properties) {
  CHECK_NE(new_routing_id, MSG_ROUTING_NONE);

  // A child frame always starts with an initial empty document, which means
  // it is in the same SiteInstance as the parent frame. Ensure that the process
  // which requested a child frame to be added is the same as the process of the
  // parent node.
  if (parent->current_frame_host()->GetProcess()->GetID() != process_id)
    return false;

  // AddChild is what creates the RenderFrameHost.
  FrameTreeNode* added_node = parent->AddChild(
      make_scoped_ptr(new FrameTreeNode(
          this, parent->navigator(), render_frame_delegate_,
          render_view_delegate_, render_widget_delegate_, manager_delegate_,
          scope, frame_name, sandbox_flags, frame_owner_properties)),
      process_id, new_routing_id);

  // Now that the new node is part of the FrameTree and has a RenderFrameHost,
  // we can announce the creation of the initial RenderFrame which already
  // exists in the renderer process.
  added_node->current_frame_host()->SetRenderFrameCreated(true);
  return true;
}

void FrameTree::RemoveFrame(FrameTreeNode* child) {
  FrameTreeNode* parent = child->parent();
  if (!parent) {
    NOTREACHED() << "Unexpected RemoveFrame call for main frame.";
    return;
  }

  parent->RemoveChild(child);
}

void FrameTree::CreateProxiesForSiteInstance(
    FrameTreeNode* source,
    SiteInstance* site_instance) {
  // Create the swapped out RVH for the new SiteInstance. This will create
  // a top-level swapped out RFH as well, which will then be wrapped by a
  // RenderFrameProxyHost.
  if (!source || !source->IsMainFrame()) {
    RenderViewHostImpl* render_view_host = GetRenderViewHost(site_instance);
    if (!render_view_host) {
      if (SiteIsolationPolicy::IsSwappedOutStateForbidden()) {
        root()->render_manager()->CreateRenderFrameProxy(site_instance);
      } else {
        root()->render_manager()->CreateRenderFrame(
            site_instance, CREATE_RF_SWAPPED_OUT | CREATE_RF_HIDDEN, nullptr);
      }
    } else {
      root()->render_manager()->EnsureRenderViewInitialized(render_view_host,
                                                            site_instance);
    }
  }

  scoped_refptr<SiteInstance> instance(site_instance);

  // Proxies are created in the FrameTree in response to a node navigating to a
  // new SiteInstance. Since |source|'s navigation will replace the currently
  // loaded document, the entire subtree under |source| will be removed.
  ForEach(base::Bind(&CreateProxyForSiteInstance, instance), source);
}

RenderFrameHostImpl* FrameTree::GetMainFrame() const {
  return root_->current_frame_host();
}

FrameTreeNode* FrameTree::GetFocusedFrame() {
  return FindByID(focused_frame_tree_node_id_);
}

void FrameTree::SetFocusedFrame(FrameTreeNode* node, SiteInstance* source) {
  if (node == GetFocusedFrame())
    return;

  std::set<SiteInstance*> frame_tree_site_instances;
  ForEach(base::Bind(&CollectSiteInstances, &frame_tree_site_instances));

  SiteInstance* current_instance =
      node->current_frame_host()->GetSiteInstance();

  // Update the focused frame in all other SiteInstances.  If focus changes to
  // a cross-process frame, this allows the old focused frame's renderer
  // process to clear focus from that frame and fire blur events.  It also
  // ensures that the latest focused frame is available in all renderers to
  // compute document.activeElement.
  //
  // We do not notify the |source| SiteInstance because it already knows the
  // new focused frame (since it initiated the focus change), and we notify the
  // new focused frame's SiteInstance (if it differs from |source|) separately
  // below.
  for (const auto& instance : frame_tree_site_instances) {
    if (instance != source && instance != current_instance) {
      DCHECK(SiteIsolationPolicy::AreCrossProcessFramesPossible());
      RenderFrameProxyHost* proxy =
          node->render_manager()->GetRenderFrameProxyHost(instance);
      proxy->SetFocusedFrame();
    }
  }

  // If |node| was focused from a cross-process frame (i.e., via
  // window.focus()), tell its RenderFrame that it should focus.
  if (current_instance != source)
    node->current_frame_host()->SetFocusedFrame();

  focused_frame_tree_node_id_ = node->frame_tree_node_id();
  node->DidFocus();
}

void FrameTree::SetFrameRemoveListener(
    const base::Callback<void(RenderFrameHost*)>& on_frame_removed) {
  on_frame_removed_ = on_frame_removed;
}

RenderViewHostImpl* FrameTree::CreateRenderViewHost(
    SiteInstance* site_instance,
    int32_t routing_id,
    int32_t main_frame_routing_id,
    bool swapped_out,
    bool hidden) {
  RenderViewHostMap::iterator iter =
      render_view_host_map_.find(site_instance->GetId());
  if (iter != render_view_host_map_.end()) {
    // If a RenderViewHost is pending deletion for this |site_instance|, it
    // shouldn't be reused, so put it in the map of RenderViewHosts pending
    // shutdown.  Otherwise, return the existing RenderViewHost for the
    // SiteInstance.  Note that if swapped-out is forbidden, the
    // RenderViewHost's main frame has already been cleared, so we cannot rely
    // on checking whether the main frame is pending deletion.
    if (iter->second->is_pending_deletion()) {
      render_view_host_pending_shutdown_map_.insert(
          std::make_pair(site_instance->GetId(), iter->second));
      render_view_host_map_.erase(iter);
    } else {
      return iter->second;
    }
  }
  RenderViewHostImpl* rvh =
      static_cast<RenderViewHostImpl*>(RenderViewHostFactory::Create(
          site_instance, render_view_delegate_, render_widget_delegate_,
          routing_id, main_frame_routing_id, swapped_out, hidden));

  render_view_host_map_[site_instance->GetId()] = rvh;
  return rvh;
}

RenderViewHostImpl* FrameTree::GetRenderViewHost(SiteInstance* site_instance) {
  RenderViewHostMap::iterator iter =
      render_view_host_map_.find(site_instance->GetId());
  if (iter == render_view_host_map_.end())
    return nullptr;
  return iter->second;
}

void FrameTree::AddRenderViewHostRef(RenderViewHostImpl* render_view_host) {
  SiteInstance* site_instance = render_view_host->GetSiteInstance();
  RenderViewHostMap::iterator iter =
      render_view_host_map_.find(site_instance->GetId());
  CHECK(iter != render_view_host_map_.end());
  CHECK(iter->second == render_view_host);

  iter->second->increment_ref_count();
}

void FrameTree::ReleaseRenderViewHostRef(RenderViewHostImpl* render_view_host) {
  SiteInstance* site_instance = render_view_host->GetSiteInstance();
  int32_t site_instance_id = site_instance->GetId();
  RenderViewHostMap::iterator iter =
      render_view_host_map_.find(site_instance_id);
  if (iter != render_view_host_map_.end() && iter->second == render_view_host) {
    // Decrement the refcount and shutdown the RenderViewHost if no one else is
    // using it.
    CHECK_GT(iter->second->ref_count(), 0);
    iter->second->decrement_ref_count();
    if (iter->second->ref_count() == 0) {
      iter->second->ShutdownAndDestroy();
      render_view_host_map_.erase(iter);
    }
  } else {
    // The RenderViewHost should be in the list of RenderViewHosts pending
    // shutdown.
    bool render_view_host_found = false;
    std::pair<RenderViewHostMultiMap::iterator,
              RenderViewHostMultiMap::iterator> result =
        render_view_host_pending_shutdown_map_.equal_range(site_instance_id);
    for (RenderViewHostMultiMap::iterator multi_iter = result.first;
         multi_iter != result.second;
         ++multi_iter) {
      if (multi_iter->second != render_view_host)
        continue;
      render_view_host_found = true;
      // Decrement the refcount and shutdown the RenderViewHost if no one else
      // is using it.
      CHECK_GT(render_view_host->ref_count(), 0);
      render_view_host->decrement_ref_count();
      if (render_view_host->ref_count() == 0) {
        render_view_host->ShutdownAndDestroy();
        render_view_host_pending_shutdown_map_.erase(multi_iter);
      }
      break;
    }
    CHECK(render_view_host_found);
  }
}

void FrameTree::FrameRemoved(FrameTreeNode* frame) {
  if (frame->frame_tree_node_id() == focused_frame_tree_node_id_)
    focused_frame_tree_node_id_ = -1;

  // No notification for the root frame.
  if (!frame->parent()) {
    CHECK_EQ(frame, root_);
    return;
  }

  // Notify observers of the frame removal.
  if (!on_frame_removed_.is_null())
    on_frame_removed_.Run(frame->current_frame_host());
}

void FrameTree::UpdateLoadProgress() {
  double progress = 0.0;
  int frame_count = 0;

  ForEach(base::Bind(&CollectLoadProgress, &progress, &frame_count));
  if (frame_count != 0)
    progress /= frame_count;

  if (progress <= load_progress_)
    return;
  load_progress_ = progress;

  // Notify the WebContents.
  root_->navigator()->GetDelegate()->DidChangeLoadProgress();
}

void FrameTree::ResetLoadProgress() {
  ForEach(base::Bind(&ResetNodeLoadProgress));
  load_progress_ = 0.0;
}

bool FrameTree::IsLoading() {
  bool is_loading = false;
  ForEach(base::Bind(&IsNodeLoading, &is_loading));
  return is_loading;
}

void FrameTree::ReplicatePageFocus(bool is_focused) {
  std::set<SiteInstance*> frame_tree_site_instances;
  ForEach(base::Bind(&CollectSiteInstances, &frame_tree_site_instances));

  // Send the focus update to main frame's proxies in all SiteInstances of
  // other frames in this FrameTree. Note that the main frame might also know
  // about proxies in SiteInstances for frames in a different FrameTree (e.g.,
  // for window.open), so we can't just iterate over its proxy_hosts_ in
  // RenderFrameHostManager.
  for (const auto& instance : frame_tree_site_instances)
    SetPageFocus(instance, is_focused);
}

void FrameTree::SetPageFocus(SiteInstance* instance, bool is_focused) {
  RenderFrameHostManager* root_manager = root_->render_manager();

  // This is only used to set page-level focus in cross-process subframes, and
  // requests to set focus in main frame's SiteInstance are ignored.
  if (instance != root_manager->current_frame_host()->GetSiteInstance()) {
    RenderFrameProxyHost* proxy =
        root_manager->GetRenderFrameProxyHost(instance);
    proxy->Send(new InputMsg_SetFocus(proxy->GetRoutingID(), is_focused));
  }
}

}  // namespace content
