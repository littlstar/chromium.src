// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_DISCOVERY_SESSION_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_DISCOVERY_SESSION_H_

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace device {

class BluetoothAdapter;

// BluetoothDiscoverySession represents a current active or inactive device
// discovery session. Instances of this class are obtained by calling
// BluetoothAdapter::StartDiscoverySession. The Bluetooth adapter will be
// constantly searching for nearby devices, as long as at least one instance
// of an active BluetoothDiscoverySession exists. A BluetoothDiscoverySession is
// considered active, as long as the adapter is discovering AND the owner of the
// instance has not called BluetoothDiscoverySession::Stop. A
// BluetoothDiscoverySession might unexpectedly become inactive, if the adapter
// unexpectedly stops discovery. Users can implement the
// AdapterDiscoveringChanged method of the BluetoothAdapter::Observer interface
// to be notified of such a change and promptly request a new
// BluetoothDiscoverySession if their existing sessions have become inactive.
class BluetoothDiscoverySession {
 public:
  // The ErrorCallback is used by methods to asynchronously report errors.
  typedef base::Closure ErrorCallback;

  // Destructor automatically terminates the discovery session. If this
  // results in a call to the underlying system to stop device discovery
  // (i.e. this instance represents the last active discovery session),
  // the call may not always succeed. To be notified of such failures,
  // users are highly encouraged to call BluetoothDiscoverySession::Stop,
  // instead of relying on the destructor.
  ~BluetoothDiscoverySession();

  // Returns true if the session is active, false otherwise. If false, the
  // adapter might still be discovering as there might still be other active
  // sessions; this just means that this instance no longer has a say in
  // whether or not discovery should continue. In this case, the application
  // should request a new BluetoothDiscoverySession to make sure that device
  // discovery continues.
  bool active() const { return active_; }

  // Requests this discovery session instance to stop. If this instance is
  // active, the session will stop. On success, |callback| is called and
  // on error |error_callback| is called. After a successful invocation, the
  // adapter may or may not stop device discovery, depending on whether or not
  // other active discovery sessions are present. Users are highly encouraged
  // to call this method to end a discovery session, instead of relying on the
  // destructor, so that they can be notified of the result via the callback
  // arguments.
  void Stop(const base::Closure& callback,
            const ErrorCallback& error_callback);

 private:
  friend class BluetoothAdapter;
  explicit BluetoothDiscoverySession(BluetoothAdapter* adapter);

  // Internal callback invoked when a call to Stop has succeeded.
  void OnStop(const base::Closure& callback);

  // Marks this instance as inactive. Called by BluetoothAdapter to mark a
  // session as inactive in the case of an unexpected change to the adapter
  // discovery state.
  void MarkAsInactive();

  // Whether or not this instance represents an active discovery session.
  bool active_;

  // The adapter that created this instance.
  scoped_refptr<BluetoothAdapter> adapter_;

  // Note: This should remain the last member so it'll be destroyed and
  // invalidate its weak pointers before any other members are destroyed.
  base::WeakPtrFactory<BluetoothDiscoverySession> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothDiscoverySession);
};

}  // namespace device

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_DISCOVERY_SESSION_H_
