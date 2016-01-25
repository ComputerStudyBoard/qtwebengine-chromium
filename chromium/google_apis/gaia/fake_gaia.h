// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GOOGLE_APIS_GAIA_FAKE_GAIA_H_
#define GOOGLE_APIS_GAIA_FAKE_GAIA_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "url/gurl.h"

namespace base {
class DictionaryValue;
}

namespace net {
namespace test_server {
class BasicHttpResponse;
struct HttpRequest;
class HttpResponse;
}
}

// This is a test helper that implements a fake GAIA service for use in browser
// tests. It's mainly intended for use with EmbeddedTestServer, for which it can
// be registered as an additional request handler.
class FakeGaia {
 public:
  using ScopeSet = std::set<std::string>;
  using RefreshTokenToDeviceIdMap = std::map<std::string, std::string>;

  // Access token details used for token minting and the token info endpoint.
  struct AccessTokenInfo {
    AccessTokenInfo();
    ~AccessTokenInfo();

    std::string token;
    std::string issued_to;
    std::string audience;
    std::string user_id;
    ScopeSet scopes;
    int expires_in;
    std::string email;
  };

  // Cookies and tokens for /MergeSession call seqeunce.
  struct MergeSessionParams {
    MergeSessionParams();
    ~MergeSessionParams();

    // Updates params with non-empty values from |params|.
    void Update(const MergeSessionParams& params);

    // Values of SID and LSID cookie that are set by /ServiceLoginAuth or its
    // equivalent at the end of the SAML login flow.
    std::string auth_sid_cookie;
    std::string auth_lsid_cookie;

    // auth_code cookie value response for /o/oauth2/programmatic_auth call.
    std::string auth_code;

    // OAuth2 refresh and access token generated by /oauth2/v4/token call
    // with "...&grant_type=authorization_code".
    std::string refresh_token;
    std::string access_token;

    // Uber token response from /OAuthLogin call.
    std::string gaia_uber_token;

    // Values of SID and LSID cookie generated from /MergeSession call.
    std::string session_sid_cookie;
    std::string session_lsid_cookie;

    // The e-mail address returned by /ListAccounts.
    std::string email;
  };

  FakeGaia();
  virtual ~FakeGaia();

  void SetFakeMergeSessionParams(const std::string& email,
                                 const std::string& auth_sid_cookie,
                                 const std::string& auth_lsid_cookie);

  // Sets the initial value of tokens and cookies.
  void SetMergeSessionParams(const MergeSessionParams& params);

  // Updates various params with non-empty values from |params|.
  void UpdateMergeSessionParams(const MergeSessionParams& params);

  // Sets the specified |gaia_id| as corresponding to the given |email|
  // address when setting GAIA response headers.  If no mapping is given for
  // an email address, a default GAIA Id is used.
  void MapEmailToGaiaId(const std::string& email, const std::string& gaia_id);

  // Initializes HTTP request handlers. Should be called after switches
  // for tweaking GaiaUrls are in place.
  void Initialize();

  // Handles a request and returns a response if the request was recognized as a
  // GAIA request. Note that this respects the switches::kGaiaUrl and friends so
  // that this can used with EmbeddedTestServer::RegisterRequestHandler().
  scoped_ptr<net::test_server::HttpResponse> HandleRequest(
      const net::test_server::HttpRequest& request);

  // Configures an OAuth2 token that'll be returned when a client requests an
  // access token for the given auth token, which can be a refresh token or an
  // login-scoped access token for the token minting endpoint. Note that the
  // scope and audience requested by the client need to match the token_info.
  void IssueOAuthToken(const std::string& auth_token,
                       const AccessTokenInfo& token_info);

  // Associates an account id with a SAML IdP redirect endpoint. When a
  // /ServiceLoginAuth request comes in for that user, it will be redirected
  // to the associated redirect endpoint.
  void RegisterSamlUser(const std::string& account_id, const GURL& saml_idp);

  void set_issue_oauth_code_cookie(bool value) {
    issue_oauth_code_cookie_ = value;
  }

  // Extracts the parameter named |key| from |query| and places it in |value|.
  // Returns false if no parameter is found.
  static bool GetQueryParameter(const std::string& query,
                                const std::string& key,
                                std::string* value);

  // Returns a device ID associated with a given |refresh_token|.
  std::string GetDeviceIdByRefreshToken(const std::string& refresh_token) const;

  void SetRefreshTokenToDeviceIdMap(
      const RefreshTokenToDeviceIdMap& refresh_token_to_device_id_map);

  const RefreshTokenToDeviceIdMap& refresh_token_to_device_id_map() const {
    return refresh_token_to_device_id_map_;
  }

  // Returns an email that is filled into the the Email field (if any).
  const std::string& prefilled_email() { return prefilled_email_; }

 protected:
  // HTTP handler for /MergeSession.
  virtual void HandleMergeSession(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);

 private:
  typedef std::multimap<std::string, AccessTokenInfo> AccessTokenInfoMap;
  typedef std::map<std::string, std::string> EmailToGaiaIdMap;
  typedef std::map<std::string, GURL> SamlAccountIdpMap;

  std::string GetGaiaIdOfEmail(const std::string& email) const;

  void AddGoogleAccountsSigninHeader(
      net::test_server::BasicHttpResponse* http_response,
      const std::string& email) const;

  void SetOAuthCodeCookie(
      net::test_server::BasicHttpResponse* http_response) const;

  // Formats a JSON response with the data in |response_dict|.
  void FormatJSONResponse(const base::DictionaryValue& response_dict,
                          net::test_server::BasicHttpResponse* http_response);

  typedef base::Callback<void(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response)>
          HttpRequestHandlerCallback;
  typedef std::map<std::string, HttpRequestHandlerCallback> RequestHandlerMap;

  // HTTP request handlers.
  void HandleProgramaticAuth(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);
  void HandleServiceLogin(const net::test_server::HttpRequest& request,
                          net::test_server::BasicHttpResponse* http_response);
  void HandleEmbeddedSetupChromeos(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);
  void HandleOAuthLogin(const net::test_server::HttpRequest& request,
                        net::test_server::BasicHttpResponse* http_response);
  void HandleServiceLoginAuth(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);
  void HandleEmbeddedLookupAccountLookup(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);
  void HandleEmbeddedSigninChallenge(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);
  void HandleSSO(const net::test_server::HttpRequest& request,
                 net::test_server::BasicHttpResponse* http_response);
  void HandleDummySAMLContinue(
      const net::test_server::HttpRequest& request,
      net::test_server::BasicHttpResponse* http_response);
  void HandleAuthToken(const net::test_server::HttpRequest& request,
                       net::test_server::BasicHttpResponse* http_response);
  void HandleTokenInfo(const net::test_server::HttpRequest& request,
                       net::test_server::BasicHttpResponse* http_response);
  void HandleIssueToken(const net::test_server::HttpRequest& request,
                        net::test_server::BasicHttpResponse* http_response);
  void HandleListAccounts(const net::test_server::HttpRequest& request,
                          net::test_server::BasicHttpResponse* http_response);
  void HandlePeopleGet(const net::test_server::HttpRequest& request,
                       net::test_server::BasicHttpResponse* http_response);
  void HandleGetUserInfo(const net::test_server::HttpRequest& request,
                         net::test_server::BasicHttpResponse* http_response);
  void HandleOAuthUserInfo(const net::test_server::HttpRequest& request,
                           net::test_server::BasicHttpResponse* http_response);

  // Returns the access token associated with |auth_token| that matches the
  // given |client_id| and |scope_string|. If |scope_string| is empty, the first
  // token satisfying the other criteria is returned. Returns NULL if no token
  // matches.
  const AccessTokenInfo* FindAccessTokenInfo(
      const std::string& auth_token,
      const std::string& client_id,
      const std::string& scope_string) const;

  // Returns the access token identified by |access_token| or NULL if not found.
  const AccessTokenInfo* GetAccessTokenInfo(
      const std::string& access_token) const;

  MergeSessionParams merge_session_params_;
  EmailToGaiaIdMap email_to_gaia_id_map_;
  AccessTokenInfoMap access_token_info_map_;
  RequestHandlerMap request_handlers_;
  std::string service_login_response_;
  std::string embedded_setup_chromeos_response_;
  SamlAccountIdpMap saml_account_idp_map_;
  bool issue_oauth_code_cookie_;
  RefreshTokenToDeviceIdMap refresh_token_to_device_id_map_;
  std::string prefilled_email_;

  DISALLOW_COPY_AND_ASSIGN(FakeGaia);
};

#endif  // GOOGLE_APIS_GAIA_FAKE_GAIA_H_
