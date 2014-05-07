// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/shell/common/shell_extensions_client.h"

#include "apps/shell/common/api/generated_schemas.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "chrome/common/extensions/api/generated_schemas.h"
#include "chrome/common/extensions/permissions/chrome_api_permissions.h"
#include "extensions/common/api/generated_schemas.h"
#include "extensions/common/api/sockets/sockets_manifest_handler.h"
#include "extensions/common/common_manifest_handlers.h"
#include "extensions/common/features/api_feature.h"
#include "extensions/common/features/base_feature_provider.h"
#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/features/manifest_feature.h"
#include "extensions/common/features/permission_feature.h"
#include "extensions/common/features/simple_feature.h"
#include "extensions/common/manifest_handler.h"
#include "extensions/common/permissions/permission_message_provider.h"
#include "extensions/common/permissions/permissions_info.h"
#include "extensions/common/permissions/permissions_provider.h"
#include "extensions/common/url_pattern_set.h"
#include "grit/app_shell_resources.h"
#include "grit/common_resources.h"
#include "grit/extensions_resources.h"

using extensions::APIPermissionInfo;
using extensions::APIPermissionSet;
using extensions::BaseFeatureProvider;
using extensions::Extension;
using extensions::FeatureProvider;
using extensions::Manifest;
using extensions::PermissionMessage;
using extensions::PermissionMessages;
using extensions::PermissionSet;
using extensions::URLPatternSet;

namespace apps {

namespace {

template <class FeatureClass>
extensions::SimpleFeature* CreateFeature() {
  return new FeatureClass;
}

// TODO(jamescook): Refactor ChromePermissionsMessageProvider so we can share
// code. For now, this implementation does nothing.
class ShellPermissionMessageProvider
    : public extensions::PermissionMessageProvider {
 public:
  ShellPermissionMessageProvider() {}
  virtual ~ShellPermissionMessageProvider() {}

  // PermissionMessageProvider implementation.
  virtual PermissionMessages GetPermissionMessages(
      const PermissionSet* permissions,
      Manifest::Type extension_type) const OVERRIDE {
    return PermissionMessages();
  }

  virtual std::vector<base::string16> GetWarningMessages(
      const PermissionSet* permissions,
      Manifest::Type extension_type) const OVERRIDE {
    return std::vector<base::string16>();
  }

  virtual std::vector<base::string16> GetWarningMessagesDetails(
      const PermissionSet* permissions,
      Manifest::Type extension_type) const OVERRIDE {
    return std::vector<base::string16>();
  }

  virtual bool IsPrivilegeIncrease(
      const PermissionSet* old_permissions,
      const PermissionSet* new_permissions,
      Manifest::Type extension_type) const OVERRIDE {
    // Ensure we implement this before shipping.
    CHECK(false);
    return false;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellPermissionMessageProvider);
};

base::LazyInstance<ShellPermissionMessageProvider>
    g_permission_message_provider = LAZY_INSTANCE_INITIALIZER;

}  // namespace

ShellExtensionsClient::ShellExtensionsClient()
    : chrome_api_permissions_(extensions::ChromeAPIPermissions()),
      extensions_api_permissions_(extensions::ExtensionsAPIPermissions()) {
}

ShellExtensionsClient::~ShellExtensionsClient() {
}

void ShellExtensionsClient::Initialize() {
  extensions::RegisterCommonManifestHandlers();

  // TODO(rockot): API manifest handlers which move out to src/extensions
  // should either end up in RegisterCommonManifestHandlers or some new
  // initialization step specifically for API manifest handlers.
  (new extensions::SocketsManifestHandler)->Register();

  extensions::ManifestHandler::FinalizeRegistration();
  // TODO(jamescook): Do we need to whitelist any extensions?

  extensions::PermissionsInfo::GetInstance()->AddProvider(
      chrome_api_permissions_);
  extensions::PermissionsInfo::GetInstance()->AddProvider(
      extensions_api_permissions_);
}

const extensions::PermissionMessageProvider&
ShellExtensionsClient::GetPermissionMessageProvider() const {
  NOTIMPLEMENTED();
  return g_permission_message_provider.Get();
}

scoped_ptr<FeatureProvider> ShellExtensionsClient::CreateFeatureProvider(
    const std::string& name) const {
  extensions::JSONFeatureProviderSource source(name);
  if (name == "api") {
    source.LoadJSON(IDR_EXTENSION_API_FEATURES);
    source.LoadJSON(IDR_SHELL_EXTENSION_API_FEATURES);
    // TODO(yoz): Don't include Chrome resources.
    source.LoadJSON(IDR_CHROME_EXTENSION_API_FEATURES);
    return scoped_ptr<FeatureProvider>(new BaseFeatureProvider(
        source.dictionary(), CreateFeature<extensions::APIFeature>));
  } else if (name == "manifest") {
    source.LoadJSON(IDR_EXTENSION_MANIFEST_FEATURES);
    source.LoadJSON(IDR_CHROME_EXTENSION_MANIFEST_FEATURES);
    return scoped_ptr<FeatureProvider>(new BaseFeatureProvider(
        source.dictionary(), CreateFeature<extensions::ManifestFeature>));
  } else if (name == "permission") {
    source.LoadJSON(IDR_EXTENSION_PERMISSION_FEATURES);
    source.LoadJSON(IDR_CHROME_EXTENSION_PERMISSION_FEATURES);
    return scoped_ptr<FeatureProvider>(new BaseFeatureProvider(
        source.dictionary(), CreateFeature<extensions::PermissionFeature>));
  } else {
    NOTREACHED();
  }
  return scoped_ptr<FeatureProvider>();
}

void ShellExtensionsClient::FilterHostPermissions(
    const URLPatternSet& hosts,
    URLPatternSet* new_hosts,
    std::set<PermissionMessage>* messages) const {
  NOTIMPLEMENTED();
}

void ShellExtensionsClient::SetScriptingWhitelist(
    const ScriptingWhitelist& whitelist) {
  scripting_whitelist_ = whitelist;
}

const extensions::ExtensionsClient::ScriptingWhitelist&
ShellExtensionsClient::GetScriptingWhitelist() const {
  // TODO(jamescook): Real whitelist.
  return scripting_whitelist_;
}

URLPatternSet ShellExtensionsClient::GetPermittedChromeSchemeHosts(
    const Extension* extension,
    const APIPermissionSet& api_permissions) const {
  NOTIMPLEMENTED();
  return URLPatternSet();
}

bool ShellExtensionsClient::IsScriptableURL(const GURL& url,
                                            std::string* error) const {
  NOTIMPLEMENTED();
  return true;
}

bool ShellExtensionsClient::IsAPISchemaGenerated(
    const std::string& name) const {
  // TODO(rockot): Remove dependency on src/chrome once we have some core APIs
  // moved out. See http://crbug.com/349042.
  // Special-case our simplified app.runtime implementation because we don't
  // have the Chrome app APIs available.
  return extensions::api::GeneratedSchemas::IsGenerated(name) ||
         extensions::core_api::GeneratedSchemas::IsGenerated(name) ||
         apps::shell_api::GeneratedSchemas::IsGenerated(name);
}

base::StringPiece ShellExtensionsClient::GetAPISchema(
    const std::string& name) const {
  // TODO(rockot): Remove dependency on src/chrome once we have some core APIs
  // moved out. See http://crbug.com/349042.
  if (extensions::api::GeneratedSchemas::IsGenerated(name))
    return extensions::api::GeneratedSchemas::Get(name);

  // Schema for chrome.shell APIs.
  if (apps::shell_api::GeneratedSchemas::IsGenerated(name))
    return apps::shell_api::GeneratedSchemas::Get(name);

  return extensions::core_api::GeneratedSchemas::Get(name);
}

bool ShellExtensionsClient::ShouldSuppressFatalErrors() const { return true; }

}  // namespace apps
