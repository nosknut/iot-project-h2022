import React, { useEffect, useState } from 'react'
import { API, graphqlOperation } from 'aws-amplify'
import { deleteDevice } from '../graphql/mutations'
import { listDevices } from '../graphql/queries'

import { Device } from '../API';
import { Button, FormControl, Grid, InputLabel, MenuItem, Select, Typography } from '@mui/material';
import { RealtimePlot } from '../components/RealtimePlot';
import { useNavigate } from 'react-router-dom';
import { setCurrentDevice, useCurrentDevice } from '../streams/currentDevice';

export const Dashboard = () => {
    const navigate = useNavigate()
    const currentDevice = useCurrentDevice();
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
        if (currentDevice) {
            const id = currentDevice.id
            if (id) {
                await (API.graphql(graphqlOperation(deleteDevice, { input: { id } })) as any)
                const newDevices = { ...devices };
                delete newDevices[id];
                setDevices(newDevices);
                setCurrentDevice(null);
            }
        }
    }

    return (
        <Grid container height="100%" marginX="auto" justifyItems="center" maxWidth="md" padding={1}>
            {Object.keys(devices).length === 0 ? (
                <Grid item xs={12} marginY={1}>
                    <Typography variant="h6" align="center">
                        You have no devices yet
                    </Typography>
                </Grid>
            ) : (
                <Grid item xs={12} marginY={1}>
                    <FormControl fullWidth>
                        <InputLabel id="device-select-label">Device</InputLabel>
                        <Select
                            labelId="device-select-label"
                            id="device-select"
                            value={currentDevice?.id || ""}
                            label="Device"
                            onChange={(e) => setCurrentDevice(devices[e.target.value as string])}
                        >
                            {Object.values(devices).map(device => (
                                <MenuItem key={device.id} value={device.id}>{device.name}</MenuItem>
                            ))}
                        </Select>
                    </FormControl>
                </Grid>
            )}
            <Grid item xs={12} marginY={1}>
                <Button
                    fullWidth
                    variant="contained"
                    onClick={() => navigate("/devices/add")}
                >Add New Device</Button>
            </Grid>
            {currentDevice && (
                <>
                    <Grid item xs={12} marginY={1}>
                        <Button
                            fullWidth
                            variant="contained"
                            color="secondary"
                            onClick={() => deleteCurrentDevice().catch(console.error)}
                        >Delete Device</Button>
                    </Grid>
                    <Grid item xs={12} height="100%">
                        <RealtimePlot />
                    </Grid>
                </>
            )}
        </Grid>
    )
}
