/* src/App.js */
import React, { useEffect, useState } from 'react'
import { Amplify, API, graphqlOperation } from 'aws-amplify'
import { createDevice, deleteDevice } from './graphql/mutations'
import { listDevices } from './graphql/queries'
import { withAuthenticator, Button, Heading, Text, View, WithAuthenticatorProps } from '@aws-amplify/ui-react';
import '@aws-amplify/ui-react/styles.css';

import awsExports from "./aws-exports";
import { Device } from './API';

Amplify.configure(awsExports);

const App = ({ signOut, user }: WithAuthenticatorProps) => {
  const [currentDeviceId, setCurrentDeviceId] = useState<string | null>(null);
  const [devices, setDevices] = useState<{ [id: string]: Device }>({});

  useEffect(() => {
    const fetchDevices = async () => {
      const response: any = await API.graphql(graphqlOperation(listDevices))
      setDevices(response.data?.listDevices?.items?.reduce((acc: any, device: Device) => {
        acc[device.id] = device;
        return acc;
      }, {}));
    }
    fetchDevices();
  }, [setDevices]);

  const deleteCurrentDevice = async () => {
    if (currentDeviceId) {
      await (API.graphql(graphqlOperation(deleteDevice, { input: { id: currentDeviceId } })) as any)
      const newDevices = { ...devices };
      delete newDevices[currentDeviceId];
      setDevices(newDevices);
      setCurrentDeviceId(null);
    }
  }

  const createNewDevice = async () => {
    const response: any = await API.graphql(graphqlOperation(createDevice, {
      input: {
        id: Math.random().toString(36).substring(2, 15) + Math.random().toString(36).substring(2, 15),
        name: 'New device',
        type: 'New type',
      }
    }))

    setDevices({ ...devices, [response.data.createDevice.id]: response.data.createDevice });
    setCurrentDeviceId(response.data.createDevice.id);
  }

  return (
    <View style={styles.container}>
      <Button style={styles.button} onClick={signOut}>Sign out</Button>
      <Heading level={2}>My devices</Heading>

      <select value={currentDeviceId || ""} onChange={(e) => setCurrentDeviceId(e.target.value)}>
        <option value="">Select a device</option>
        {Object.values(devices).map((device) => (
          <option key={device.id} value={device.id}>{device.name}</option>
        ))}
      </select>
      
      <button onClick={() => createNewDevice().catch(console.error)}>Add new device</button>
      {currentDeviceId && <button onClick={() => deleteCurrentDevice().catch(console.error)}>Delete current device</button>}

      {currentDeviceId && (
        <View>
          <Heading level={3}>{devices[currentDeviceId].name}</Heading>
          <Text>Device ID: {devices[currentDeviceId].id}</Text>
          <Text>Device type: {devices[currentDeviceId].type}</Text>
        </View>
      )}
    </View>
  )
}

const styles = {
  container: { width: 400, margin: '0 auto', display: 'flex', flexDirection: 'column', justifyContent: 'center', padding: 20 },
  button: { backgroundColor: 'black', color: 'white', outline: 'none', fontSize: 18, padding: '12px 0px' }
}

export default withAuthenticator(App);