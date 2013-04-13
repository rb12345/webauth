/*
 * Helper functions for testing WebAuth code.
 *
 * Additional functions that are helpful for testing WebAuth code and have
 * knowledge of WebAuth functions and data structures.  In all of the token
 * comparison functions, each component of the tokens is compared as a
 * separate test result, since that makes problem reporting much clearer and
 * more helpful to the developer.
 *
 * Written by Russ Allbery <rra@stanford.edu>
 * Copyright 2013
 *     The Board of Trustees of the Leland Stanford Junior University
 *
 * See LICENSE for licensing terms.
 */

#include <config.h>
#include <portable/apr.h>
#include <portable/system.h>

#include <lib/internal.h>
#include <tests/tap/basic.h>
#include <tests/tap/kerberos.h>
#include <tests/tap/string.h>
#include <tests/tap/webauth.h>
#include <webauth/keys.h>
#include <webauth/tokens.h>
#include <webauth/webkdc.h>


/*
 * Check a token creation time.  Takes the wanted and seen creation times, and
 * if wanted is 0, expects a creation time within a range of 5 seconds old and
 * 1 second fast compared to the current time.
 */
static void
is_token_creation(time_t wanted, time_t seen, const char *format, ...)
{
    va_list args;
    time_t now;
    bool okay;

    if (wanted == 0) {
        now = time(NULL);
        okay = (seen >= now - 5 && seen <= now + 1);
    } else {
        okay = (wanted == seen);
    }
    if (!okay)
        printf("# wanted: %lu\n#   seen: %lu\n", (unsigned long) wanted,
               (unsigned long) seen);
    va_start(args, format);
    okv(okay, format, args);
    va_end(args);
}

/*
 * Check a token expiration time.  Takes the wanted and seen expiration times,
 * and if wanted is 0, expects a creation time at any point in the future.
 */
static void
is_token_expiration(time_t wanted, time_t seen, const char *format, ...)
{
    va_list args;
    time_t now;
    bool okay;

    if (wanted == 0) {
        now = time(NULL);
        okay = (seen > now);
    } else {
        okay = (wanted == seen);
    }
    if (!okay)
        printf("# wanted: %lu\n#   seen: %lu\n", (unsigned long) wanted,
               (unsigned long) seen);
    va_start(args, format);
    okv(okay, format, args);
    va_end(args);
}


/*
 * Compare two error tokens.
 */
void
is_token_error(const struct webauth_token_error *wanted,
               const struct webauth_token_error *seen,
               const char *format, ...)
{
    va_list args;
    char *message;

    va_start(args, format);
    bvasprintf(&message, format, args);
    va_end(args);
    if (seen == NULL) {
        ok_block(3, false, "%s is NULL", message);
        return;
    }
    is_int(wanted->code, seen->code, "%s code", message);
    is_string(wanted->message, seen->message, "%s subject", message);
    is_token_creation(wanted->creation, seen->creation, "%s creation",
                      message);
    free(message);
}


/*
 * Compare two id tokens.
 */
void
is_token_id(const struct webauth_token_id *wanted,
            const struct webauth_token_id *seen,
            const char *format, ...)
{
    va_list args;
    char *message;

    va_start(args, format);
    bvasprintf(&message, format, args);
    va_end(args);
    if (seen == NULL) {
        ok_block(10, false, "%s is NULL", message);
        return;
    }
    is_string(wanted->subject, seen->subject, "%s subject", message);
    is_string(wanted->authz_subject, seen->authz_subject, "%s authz subject",
              message);
    is_string(wanted->auth, seen->auth, "%s auth type", message);
    if (wanted->auth_data == NULL || seen->auth_data == NULL)
        ok(wanted->auth_data == seen->auth_data, "%s auth data", message);
    else
        ok(memcmp(wanted->auth_data, seen->auth_data,
                  wanted->auth_data_len) == 0, "%s auth data", message);
    is_int(wanted->auth_data_len, seen->auth_data_len, "%s auth data length",
           message);
    is_string(wanted->initial_factors, seen->initial_factors,
              "%s initial factors", message);
    is_string(wanted->session_factors, seen->session_factors,
              "%s session factors", message);
    is_int(wanted->loa, seen->loa, "%s level of assurance", message);
    is_token_creation(wanted->creation, seen->creation, "%s creation",
                      message);
    is_token_expiration(wanted->expiration, seen->expiration, "%s expiration",
                        message);
    free(message);
}


/*
 * Compare two proxy tokens.
 */
void
is_token_proxy(const struct webauth_token_proxy *wanted,
               const struct webauth_token_proxy *seen,
               const char *format, ...)
{
    va_list args;
    char *message;

    va_start(args, format);
    bvasprintf(&message, format, args);
    va_end(args);
    if (seen == NULL) {
        ok_block(10, false, "%s is NULL", message);
        return;
    }
    is_string(wanted->subject, seen->subject, "%s subject", message);
    is_string(wanted->authz_subject, seen->authz_subject, "%s authz subject",
              message);
    is_string(wanted->type, seen->type, "%s proxy type", message);
    if (wanted->webkdc_proxy == NULL || seen->webkdc_proxy == NULL)
        ok(wanted->webkdc_proxy == seen->webkdc_proxy, "%s webkdc proxy",
           message);
    else
        ok(memcmp(wanted->webkdc_proxy, seen->webkdc_proxy,
                  wanted->webkdc_proxy_len) == 0, "%s webkdc proxy", message);
    is_int(wanted->webkdc_proxy_len, seen->webkdc_proxy_len,
           "%s webkdc proxy length", message);
    is_string(wanted->initial_factors, seen->initial_factors,
              "%s initial factors", message);
    is_string(wanted->session_factors, seen->session_factors,
              "%s session factors", message);
    is_int(wanted->loa, seen->loa, "%s level of assurance", message);
    is_token_creation(wanted->creation, seen->creation, "%s creation",
                      message);
    is_token_expiration(wanted->expiration, seen->expiration, "%s expiration",
                        message);
    free(message);
}


/*
 * Compare two webkdc-factor tokens.
 */
void
is_token_webkdc_factor(const struct webauth_token_webkdc_factor *wanted,
                       const struct webauth_token_webkdc_factor *seen,
                       const char *format, ...)
{
    va_list args;
    char *message;

    va_start(args, format);
    bvasprintf(&message, format, args);
    va_end(args);
    if (seen == NULL) {
        ok_block(4, false, "%s is NULL", message);
        return;
    }
    is_string(wanted->subject, seen->subject, "%s subject", message);
    is_string(wanted->factors, seen->factors, "%s factors", message);
    is_token_creation(wanted->creation, seen->creation, "%s creation",
                      message);
    is_token_expiration(wanted->expiration, seen->expiration, "%s expiration",
                        message);
    free(message);
}


/*
 * Compare two webkdc-proxy tokens.
 */
void
is_token_webkdc_proxy(const struct webauth_token_webkdc_proxy *wanted,
                      const struct webauth_token_webkdc_proxy *seen,
                      const char *format, ...)
{
    va_list args;
    char *message;

    va_start(args, format);
    bvasprintf(&message, format, args);
    va_end(args);
    if (seen == NULL) {
        ok_block(9, false, "%s is NULL", message);
        return;
    }
    is_string(wanted->subject, seen->subject, "%s subject", message);
    is_string(wanted->proxy_type, seen->proxy_type, "%s proxy type", message);
    is_string(wanted->proxy_subject, seen->proxy_subject, "%s proxy subject",
              message);
    if (wanted->data == NULL || seen->data == NULL) {
        if (strcmp(wanted->proxy_type, "krb5") == 0) {
            ok(seen->data != NULL, "%s has proxy data", message);
            ok(seen->data_len > 0, "%s has proxy data length", message);
        } else {
            ok(wanted->data == seen->data, "%s proxy data", message);
            is_int(wanted->data_len, seen->data_len, "%s proxy data length",
                   message);
        }
    } else {
        ok(memcmp(wanted->data, seen->data, wanted->data_len) == 0,
           "%s proxy data", message);
        is_int(wanted->data_len, seen->data_len, "%s proxy data length",
               message);
    }
    is_string(wanted->initial_factors, seen->initial_factors,
              "%s initial factors", message);
    is_int(wanted->loa, seen->loa, "%s level of assurance", message);
    is_token_creation(wanted->creation, seen->creation, "%s creation",
                      message);
    is_token_expiration(wanted->expiration, seen->expiration, "%s expiration",
                        message);
    is_string(wanted->session_factors, seen->session_factors,
              "%s session factors", message);
    free(message);
}


/*
 * Map a string that may contain placeholders for Kerberos identities to what
 * we actually expect in the test results.  Takes the WebAuth context (for
 * memory alloation), the string, and the Kerberos configuration (which may be
 * NULL) and returns the correct string.
 */
static const char *
subst(struct webauth_context *ctx, const char *template,
      const struct kerberos_config *krbconf)
{
    if (krbconf == NULL)
        return template;
    if (strcmp(template, "<principal>") == 0)
        return krbconf->principal;
    else if (strcmp(template, "<krb5-principal>") == 0)
        return apr_pstrcat(ctx->pool, "krb5:", krbconf->principal, (char *) 0);
    else if (strcmp(template, "<webkdc-principal>") == 0)
        return apr_pstrcat(ctx->pool, "WEBKDC:krb5:", krbconf->principal,
                           (char *) 0);
    else if (strcmp(template, "<userprinc>") == 0)
        return krbconf->userprinc;
    else if (strcmp(template, "<username>") == 0)
        return krbconf->username;
    else if (strcmp(template, "<password>") == 0)
        return krbconf->password;
    else
        return template;
}


/*
 * Internal helper function to build an id token from a wat_token_id
 * structure.  Takes the WebAuth context, the template, the time basis, and an
 * optional Kerberos configuration to replace principal placeholders, and
 * returns a newly-allocated token.
 */
static struct webauth_token *
build_token_id(struct webauth_context *ctx,
               const struct wat_token_id *template, time_t now,
               const struct kerberos_config *krbconf)
{
    struct webauth_token *token;
    struct webauth_token_id *id;

    token = apr_pcalloc(ctx->pool, sizeof(struct webauth_token));
    token->type = WA_TOKEN_ID;
    id = &token->token.id;
    id->subject         = subst(ctx, template->subject, krbconf);
    id->authz_subject   = template->authz_subject;
    id->auth            = template->auth;
    id->auth_data       = template->auth_data;
    id->auth_data_len   = template->auth_data_len;
    id->initial_factors = template->initial_factors;
    id->session_factors = template->session_factors;
    id->loa             = template->loa;
    id->creation        = template->creation;
    id->expiration      = template->expiration;
    if (id->creation < 1000 && id->creation > 0)
        id->creation = now - id->creation;
    if (id->expiration < 1000 && id->expiration > 0)
        id->expiration += now;
    return token;
}


/*
 * Internal helper function to build a login token from a wat_token_login
 * structure.  Takes the WebAuth context, the template, the time basis, and an
 * optional Kerberos configuration to replace principal placeholders, and
 * returns a newly-allocated token.
 */
static struct webauth_token *
build_token_login(struct webauth_context *ctx,
                  const struct wat_token_login *template, time_t now,
                  const struct kerberos_config *krbconf)
{
    struct webauth_token *token;
    struct webauth_token_login *login;

    token = apr_pcalloc(ctx->pool, sizeof(struct webauth_token));
    token->type = WA_TOKEN_LOGIN;
    login = &token->token.login;
    login->username = subst(ctx, template->username, krbconf);
    login->password = subst(ctx, template->password, krbconf);
    login->otp      = template->otp;
    login->otp_type = template->otp_type;
    login->creation = template->creation;
    if (login->creation < 1000 && login->creation > 0)
        login->creation = now - login->creation;
    return token;
}


/*
 * Internal helper function to build a proxy token from a wat_token_proxy
 * structure.  Takes the WebAuth context, the template, the time basis, and an
 * optional Kerberos configuration to replace principal placeholders, and
 * returns a newly-allocated token.
 */
static struct webauth_token *
build_token_proxy(struct webauth_context *ctx,
                  const struct wat_token_proxy *template, time_t now,
                  const struct kerberos_config *krbconf)
{
    struct webauth_token *token;
    struct webauth_token_proxy *proxy;

    token = apr_pcalloc(ctx->pool, sizeof(struct webauth_token));
    token->type = WA_TOKEN_PROXY;
    proxy = &token->token.proxy;
    proxy->subject         = subst(ctx, template->subject, krbconf);
    proxy->authz_subject   = template->authz_subject;
    proxy->type            = template->type;
    proxy->initial_factors = template->initial_factors;
    proxy->session_factors = template->session_factors;
    proxy->loa             = template->loa;
    proxy->creation        = template->creation;
    proxy->expiration      = template->expiration;
    if (proxy->creation < 1000 && proxy->creation > 0)
        proxy->creation = now - proxy->creation;
    if (proxy->expiration < 1000 && proxy->expiration > 0)
        proxy->expiration += now;
    return token;
}


/*
 * Internal helper function to build a webkdc-proxy token from a
 * wat_token_webkdc_proxy structure.  Takes the WebAuth context, the template,
 * the time basis, and an optional Kerberos configuration to replace principal
 * placeholders, and returns a newly-allocated token.
 */
static struct webauth_token *
build_token_webkdc_proxy(struct webauth_context *ctx,
                         const struct wat_token_webkdc_proxy *template,
                         time_t now, const struct kerberos_config *krbconf)
{
    struct webauth_token *token;
    struct webauth_token_webkdc_proxy *wkproxy;

    token = apr_pcalloc(ctx->pool, sizeof(struct webauth_token));
    token->type = WA_TOKEN_WEBKDC_PROXY;
    wkproxy = &token->token.webkdc_proxy;
    wkproxy->subject         = subst(ctx, template->subject, krbconf);
    wkproxy->proxy_type      = template->proxy_type;
    wkproxy->proxy_subject   = subst(ctx, template->proxy_subject, krbconf);
    wkproxy->data            = template->data;
    wkproxy->data_len        = template->data_len;
    wkproxy->initial_factors = template->initial_factors;
    wkproxy->loa             = template->loa;
    wkproxy->creation        = template->creation;
    wkproxy->expiration      = template->expiration;
    wkproxy->session_factors = template->session_factors;
    if (wkproxy->creation < 1000 && wkproxy->creation > 0)
        wkproxy->creation = now - wkproxy->creation;
    if (wkproxy->expiration < 1000 && wkproxy->expiration > 0)
        wkproxy->expiration += now;
    return token;
}


/*
 * Internal helper function to build a webkdc-service token from a
 * wat_token_webkdc_service structure.  Takes the WebAuth context, the
 * template, the key to use as the session key, the time basis, and an
 * optional Kerberos configuration to replace principal placeholders, and
 * returns a newly-allocated token.
 */
static struct webauth_token *
build_token_webkdc_service(struct webauth_context *ctx,
                           const struct wat_token_webkdc_service *template,
                           const struct webauth_key *key, time_t now,
                           const struct kerberos_config *krbconf)
{
    struct webauth_token *token;
    struct webauth_token_webkdc_service *wkservice;

    token = apr_pcalloc(ctx->pool, sizeof(struct webauth_token));
    token->type = WA_TOKEN_WEBKDC_SERVICE;
    wkservice = &token->token.webkdc_service;
    wkservice->subject         = subst(ctx, template->subject, krbconf);
    wkservice->session_key     = key->data;
    wkservice->session_key_len = key->length;
    wkservice->creation        = template->creation;
    wkservice->expiration      = template->expiration;
    if (wkservice->creation < 1000 && wkservice->creation > 0)
        wkservice->creation = now - wkservice->creation;
    if (wkservice->expiration < 1000 && wkservice->expiration > 0)
        wkservice->expiration += now;
    return token;
}


/*
 * Validate a WebKDC login response against a struct wat_login_test.  Takes a
 * WebAuth context, the test, the response, the WebKDC and session keyrings,
 * the time basis, and an optional Kerberos configuration.
 */
static void
check_login_response(struct webauth_context *ctx,
                     const struct wat_login_test *test,
                     const struct webauth_webkdc_login_response *response,
                     const struct webauth_keyring *ring,
                     const struct webauth_keyring *session, time_t now,
                     const struct kerberos_config *krbconf)
{
    const char *factors, *options;
    struct webauth_token *token, *wanted;
    enum webauth_token_type type;
    size_t i;
    int s;

    /* The contents of any login canceled token. */
    static const struct webauth_token_error cancel_token = {
        WA_PEC_LOGIN_CANCELED, "user canceled login", 0
    };

    /* Check the login error code and message. */
    is_int(test->response.login_error, response->login_error,
           "... login error code");
    is_string(test->response.login_message, response->login_message,
              "... login error message");

    /* Check various simple response parameters. */
    is_string(test->response.user_message, response->user_message,
              "... user message");
    is_int(test->response.password_expires, response->password_expires,
           "... password expires");

    /* Check response parameters derived directly from the request. */
    is_string(test->request.request.return_url, response->return_url,
              "... return URL");
    is_string(test->request.service.subject, response->requester,
              "... requester");

    /* Check wanted and configured factors. */
    if (response->factors_wanted == NULL)
        ok(test->response.factors_wanted == NULL, "... has wanted factors");
    else {
        factors = apr_array_pstrcat(ctx->pool, response->factors_wanted, ',');
        is_string(test->response.factors_wanted, factors,
                  "... wanted factors");
    }
    if (response->factors_configured == NULL)
        ok(test->response.factors_configured == NULL,
           "... has configured factors");
    else {
        factors
            = apr_array_pstrcat(ctx->pool, response->factors_configured, ',');
        is_string(test->response.factors_configured, factors,
                  "... configured factors");
    }

    /*
     * Check returned webkdc-proxy tokens.  If and only if there is at least
     * one webkdc-proxy token, we will know and return the authenticated
     * identity in the subject field, so check that here as well.
     */
    for (i = 0; i < ARRAY_SIZE(test->response.proxies); i++) {
        struct webauth_webkdc_proxy_data *pd;

        if (test->response.proxies[i].subject == NULL)
            break;
        if (response->proxies == NULL || response->proxies->nelts <= (int) i)
            continue;
        pd = &APR_ARRAY_IDX(response->proxies, i,
                            struct webauth_webkdc_proxy_data);
        is_string(test->response.proxies[i].proxy_type, pd->type,
                  "... type of webkdc-proxy token %d", i);
        type = WA_TOKEN_WEBKDC_PROXY;
        s = webauth_token_decode(ctx, type, pd->token, ring, &token);
        is_int(WA_ERR_NONE, s, "... webkdc-proxy %d decodes", i);
        wanted = build_token_webkdc_proxy(ctx, &test->response.proxies[i],
                                          now, krbconf);
        is_token_webkdc_proxy(&wanted->token.webkdc_proxy,
                              &token->token.webkdc_proxy,
                              "... webkdc-proxy %d", i);
    }
    if (i == 0) {
        ok(response->proxies == NULL, "... has no webkdc-proxy tokens");
        is_string(NULL, response->subject, "... subject");
    } else if (response->proxies == NULL) {
        is_int(i, 0, "... correct number of webkdc-proxy tokens");
        is_string(subst(ctx, test->response.proxies[0].subject, krbconf),
                  response->subject, "... subject");
    } else {
        is_int(i, response->proxies->nelts,
               "... correct number of webkdc-proxy tokens");
        is_string(subst(ctx, test->response.proxies[0].subject, krbconf),
                  response->subject, "... subject");
    }

    /*
     * Check returned webkdc-factor tokens.  While we return a list for
     * forward-compatibility, the WebKDC will currently only ever return a
     * single token, which is reflected in the structure of the test data.
     */
    if (test->response.factor_token.subject == NULL)
        ok(response->factor_tokens == NULL, "... has no webkdc-factor tokens");
    else if (response->factor_tokens == NULL)
        ok(false, "... webkdc-factor token");
    else {
        struct webauth_webkdc_factor_data *fd;

        is_int(1, response->factor_tokens->nelts,
               "... one webkdc-factor token");
        fd = &APR_ARRAY_IDX(response->factor_tokens, 0,
                            struct webauth_webkdc_factor_data);
        is_int(test->response.factor_token.expiration, fd->expiration,
               "... expiration of webkdc-factor token");
        type = WA_TOKEN_WEBKDC_FACTOR;
        s = webauth_token_decode(ctx, type, fd->token, ring, &token);
        is_int(WA_ERR_NONE, s, "... webkdc-factor %d decodes", i);
        is_token_webkdc_factor(&test->response.factor_token,
                               &token->token.webkdc_factor,
                               "... webkdc-factor");
    }

    /*
     * Check the result token.  We determine which result type we're expecting
     * based on whether result_id or result_proxy has a non-NULL subject.  We
     * also check various other information in the response that's based on
     * the result token.
     */
    if (test->response.result_id.subject != NULL) {
        is_string("id", response->result_type, "... result type");
        is_string(test->response.result_id.authz_subject,
                  response->authz_subject, "... authorization subject");
        is_string(test->response.result_id.initial_factors,
                  response->initial_factors, "... initial factors");
        is_string(test->response.result_id.session_factors,
                  response->session_factors, "... session factors");
        is_int(test->response.result_id.loa, response->loa,
               "... level of assurance");
        type = WA_TOKEN_ID;
    } else if (test->response.result_proxy.subject != NULL) {
        is_string("proxy", response->result_type, "... result type");
        is_string(test->response.result_proxy.authz_subject,
                  response->authz_subject, "... authorization subject");
        is_string(test->response.result_proxy.initial_factors,
                  response->initial_factors, "... initial factors");
        is_string(test->response.result_proxy.session_factors,
                  response->session_factors, "... session factors");
        is_int(test->response.result_proxy.loa, response->loa,
               "... level of assurance");
        type = WA_TOKEN_PROXY;
    } else {
        is_string(NULL, response->result_type, "... result type");
        ok(response->result == NULL, "... no result token");
        is_string(NULL, response->authz_subject, "... authorization subject");
        is_string(NULL, response->initial_factors, "... initial factors");
        is_string(NULL, response->session_factors, "... session factors");
        is_int(0, response->loa, "... level of assurance");
        type = WA_TOKEN_UNKNOWN;
    }
    if (type != WA_TOKEN_UNKNOWN && response->result != NULL) {
        s = webauth_token_decode(ctx, type, response->result, session, &token);
        is_int(WA_ERR_NONE, s, "... result token decodes");
        if (type == WA_TOKEN_ID) {
            wanted = build_token_id(ctx, &test->response.result_id, now,
                                    krbconf);
            is_token_id(&wanted->token.id, &token->token.id, "... result");
        } else if (type == WA_TOKEN_PROXY) {
            wanted = build_token_proxy(ctx, &test->response.result_proxy, now,
                                       krbconf);
            is_token_proxy(&wanted->token.proxy, &token->token.proxy,
                           "... result");
        }
    }

    /* Check the login cancel token. */
    options = test->request.request.options;
    if (options == NULL || strstr(options, "lc") == NULL)
        ok(response->login_cancel == NULL, "... no login cancel token");
    else if (response->login_cancel == NULL)
        ok(false, "... login cancel token");
    else {
        const char *cancel;

        type = WA_TOKEN_ERROR;
        cancel = response->login_cancel;
        s = webauth_token_decode(ctx, type, cancel, session, &token);
        is_int(WA_ERR_NONE, s, "... login cancel token decodes");
        is_token_error(&cancel_token, &token->token.error,
                       "... login cancel token");
    }

    /* Check the application state. */
    if (test->request.request.state == NULL) {
        ok(response->app_state == NULL, "... no application state");
        is_int(0, response->app_state_len, "... application state length");
    } else {
        ok(memcmp(test->request.request.state, response->app_state,
                  test->request.request.state_len) == 0,
           "... application state data");
        is_int(test->request.request.state_len, response->app_state_len,
               "... application state length");
    }

    /* Check the login data. */
    for (i = 0; i < ARRAY_SIZE(test->response.logins); i++) {
        struct webauth_login *login;

        if (test->response.logins[i].ip == NULL)
            break;
        if (response->logins == NULL || response->logins->nelts <= (int) i)
            continue;
        login = &APR_ARRAY_IDX(response->logins, i, struct webauth_login);
        is_string(test->response.logins[i].ip, login->ip,
                  "... login %d ip", i);
        is_string(test->response.logins[i].hostname, login->hostname,
                  "... login %d hostname", i);
        is_int(test->response.logins[i].timestamp, login->timestamp,
               "... login %d timestamp", i);
    }
    if (i == 0)
        ok(response->logins == NULL, "... has no login information");
    else if (response->logins == NULL)
        is_int(i, 0, "... correct number of login records");
    else
        is_int(i, response->logins->nelts,
               "... correct number of login records");

    /* Check the permitted authorization identities. */
    for (i = 0; i < ARRAY_SIZE(test->response.permitted_authz); i++) {
        const char *authz;

        if (test->response.permitted_authz[i] == NULL)
            break;
        if (response->permitted_authz == NULL)
            continue;
        if (response->permitted_authz->nelts <= (int) i)
            continue;
        authz = APR_ARRAY_IDX(response->permitted_authz, i, const char *);
        is_string(test->response.permitted_authz[i], authz,
                  "... permitted authz %d", i);
    }
    if (i == 0)
        ok(response->permitted_authz == NULL, "... has no permitted authz");
    else if (response->permitted_authz == NULL)
        is_int(i, 0, "... correct number of permitted_authz");
    else
        is_int(i, response->permitted_authz->nelts,
               "... correct number of permitted_authz");
}


/*
 * Run a test of the WebKDC login handling.  Takes the WebAuth context in
 * which to run the tests, the test case description, the keyring to use for
 * the WebKDC, and an optional Kerberos configuration.
 *
 * If the Kerberos configuration is present, it will be used to replace tokens
 * in the test data with the principal information from the Kerberos
 * configuration.
 */
void
run_login_test(struct webauth_context *ctx, const struct wat_login_test *test,
               const struct webauth_keyring *ring,
               const struct kerberos_config *krbconf)
{
    struct webauth_keyring *session;
    struct webauth_key *key;
    struct webauth_token *token;
    struct webauth_webkdc_login_request request;
    struct webauth_webkdc_login_response *response;
    apr_array_header_t *creds;
    const char *message;
    int s;
    size_t i;
    time_t now;

    /* Use a common time basis for everything that follows. */
    now = time(NULL);

    /* Start with a blank request. */
    memset(&request, 0, sizeof(request));

    /*
     * Create a template webkdc-service token with a unique key.  We will
     * modify this for each test case.
     */
    s = webauth_key_create(ctx, WA_KEY_AES, WA_AES_128, NULL, &key);
    if (s != WA_ERR_NONE)
        bail("cannot create key: %s", webauth_error_message(ctx, s));
    session = webauth_keyring_from_key(ctx, key);
    token = build_token_webkdc_service(ctx, &test->request.service, key, now,
                                       krbconf);
    request.service = &token->token.webkdc_service;

    /* Create an array for credentials. */
    creds = apr_array_make(ctx->pool, 3, sizeof(struct webauth_token *));

    /* Add the login tokens to the array. */
    for (i = 0; i < ARRAY_SIZE(test->request.logins); i++) {
        if (test->request.logins[i].username == NULL)
            break;
        token = build_token_login(ctx, &test->request.logins[i], now, krbconf);
        APR_ARRAY_PUSH(creds, struct webauth_token *) = token;
    }

    /* Add the webkdc-proxy tokens to the array. */
    for (i = 0; i < ARRAY_SIZE(test->request.wkproxies); i++) {
        if (test->request.wkproxies[i].subject == NULL)
            break;
        token = build_token_webkdc_proxy(ctx, &test->request.wkproxies[i], now,
                                         krbconf);
        APR_ARRAY_PUSH(creds, struct webauth_token *) = token;
    }

    /* Add the webkdc-factor tokens to the array. */
    for (i = 0; i < ARRAY_SIZE(test->request.wkfactors); i++) {
        if (test->request.wkfactors[i].subject == NULL)
            break;
        token = apr_pcalloc(ctx->pool, sizeof(struct webauth_token));
        token->type = WA_TOKEN_WEBKDC_FACTOR;
        token->token.webkdc_factor = test->request.wkfactors[i];
        APR_ARRAY_PUSH(creds, struct webauth_token *) = token;
    }

    /* Add the remaining data to the request. */
    request.creds         = creds;
    request.request       = &test->request.request;
    request.authz_subject = test->request.authz_subject;

    /* Make the actual call. */
    s = webauth_webkdc_login(ctx, &request, &response, ring);

    /*
     * Check the WebAuth return code.  If we expect to fail, no other
     * validation is useful, so don't do anything further.
     */
    if (s != WA_ERR_NONE && test->status == WA_ERR_NONE)
        diag("%s", webauth_error_message(ctx, s));
    is_int(test->status, s, "%s (status)", test->name);
    if (test->status != WA_ERR_NONE) {
        message = webauth_error_message(ctx, s);
        is_string(test->error, message, "... and error message");
        return;
    }

    /* Check the response. */
    check_login_response(ctx, test, response, ring, session, now, krbconf);
}
