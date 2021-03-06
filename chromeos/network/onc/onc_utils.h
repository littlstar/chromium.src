// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_NETWORK_ONC_ONC_UTILS_H_
#define CHROMEOS_NETWORK_ONC_ONC_UTILS_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/network/network_type_pattern.h"
#include "components/onc/onc_constants.h"

namespace base {
class DictionaryValue;
class ListValue;
}

namespace net {
class X509Certificate;
}

namespace chromeos {
namespace onc {

struct OncValueSignature;

// A valid but empty (no networks and no certificates) and unencrypted
// configuration.
CHROMEOS_EXPORT extern const char kEmptyUnencryptedConfiguration[];

typedef std::map<std::string, std::string> CertPEMsByGUIDMap;

// Parses |json| according to the JSON format. If |json| is a JSON formatted
// dictionary, the function returns the dictionary as a DictionaryValue.
// Otherwise returns NULL.
CHROMEOS_EXPORT scoped_ptr<base::DictionaryValue> ReadDictionaryFromJson(
    const std::string& json);

// Decrypts the given EncryptedConfiguration |onc| (see the ONC specification)
// using |passphrase|. The resulting UnencryptedConfiguration is returned. If an
// error occurs, returns NULL.
CHROMEOS_EXPORT scoped_ptr<base::DictionaryValue> Decrypt(
    const std::string& passphrase,
    const base::DictionaryValue& onc);

// For logging only: strings not user facing.
CHROMEOS_EXPORT std::string GetSourceAsString(::onc::ONCSource source);

// Used for string expansion with function ExpandStringInOncObject(...).
class CHROMEOS_EXPORT StringSubstitution {
 public:
  StringSubstitution() {}
  virtual ~StringSubstitution() {}

  // Returns the replacement string for |placeholder| in
  // |substitute|. Currently, substitutes::kLoginIDField and
  // substitutes::kEmailField are supported.
  virtual bool GetSubstitute(const std::string& placeholder,
                             std::string* substitute) const = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(StringSubstitution);
};

// Replaces all expandable fields that are mentioned in the ONC
// specification. The object of |onc_object| is modified in place. Currently
// substitutes::kLoginIDField and substitutes::kEmailField are expanded. The
// replacement strings are obtained from |substitution|.
CHROMEOS_EXPORT void ExpandStringsInOncObject(
    const OncValueSignature& signature,
    const StringSubstitution& substitution,
    base::DictionaryValue* onc_object);

// Replaces expandable fields in the networks of |network_configs|, which must
// be a list of ONC NetworkConfigurations. See ExpandStringsInOncObject above.
CHROMEOS_EXPORT void ExpandStringsInNetworks(
    const StringSubstitution& substitution,
    base::ListValue* network_configs);

// Creates a copy of |onc_object| with all values of sensitive fields replaced
// by |mask|. To find sensitive fields, signature and field name are checked
// with the function FieldIsCredential().
CHROMEOS_EXPORT scoped_ptr<base::DictionaryValue> MaskCredentialsInOncObject(
    const OncValueSignature& signature,
    const base::DictionaryValue& onc_object,
    const std::string& mask);

// Decrypts |onc_blob| with |passphrase| if necessary. Clears |network_configs|,
// |global_network_config| and |certificates| and fills them with the validated
// NetworkConfigurations, GlobalNetworkConfiguration and Certificates of
// |onc_blob|. Returns false if any validation errors or warnings occurred.
// Still, some configuration might be added to the output arguments and should
// be further processed by the caller.
CHROMEOS_EXPORT bool ParseAndValidateOncForImport(
    const std::string& onc_blob,
    ::onc::ONCSource onc_source,
    const std::string& passphrase,
    base::ListValue* network_configs,
    base::DictionaryValue* global_network_config,
    base::ListValue* certificates);

// Parse the given PEM encoded certificate |pem_encoded| and create a
// X509Certificate from it.
CHROMEOS_EXPORT scoped_refptr<net::X509Certificate> DecodePEMCertificate(
    const std::string& pem_encoded);

// Replaces all references by GUID to Server or CA certs by their PEM
// encoding. Returns true if all references could be resolved. Otherwise returns
// false and network configurations with unresolveable references are removed
// from |network_configs|. |network_configs| must be a list of ONC
// NetworkConfiguration dictionaries.
CHROMEOS_EXPORT bool ResolveServerCertRefsInNetworks(
    const CertPEMsByGUIDMap& certs_by_guid,
    base::ListValue* network_configs);

// Replaces all references by GUID to Server or CA certs by their PEM
// encoding. Returns true if all references could be resolved. |network_config|
// must be a ONC NetworkConfiguration.
CHROMEOS_EXPORT bool ResolveServerCertRefsInNetwork(
    const CertPEMsByGUIDMap& certs_by_guid,
    base::DictionaryValue* network_config);

// Returns a network type pattern for matching the ONC type string.
CHROMEOS_EXPORT NetworkTypePattern NetworkTypePatternFromOncType(
    const std::string& type);

// Returns true if |property_key| is a recommended value in the ONC dictionary.
CHROMEOS_EXPORT bool IsRecommendedValue(const base::DictionaryValue* onc,
                                        const std::string& property_key);

}  // namespace onc
}  // namespace chromeos

#endif  // CHROMEOS_NETWORK_ONC_ONC_UTILS_H_
