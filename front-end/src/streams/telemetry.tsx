import { bind } from "@react-rxjs/core";
import { createSignal } from "@react-rxjs/utils";
import { combineLatestWith, distinct, filter, interval, map } from "rxjs";
import { currentDeviceId$ } from "./currentDeviceId";

const TELEMETRY_URL = "https://3n36puf5td.execute-api.us-west-2.amazonaws.com/default/ddb_rest_api"

export type TempDataEntry = {
    device_id: string
    sample_time: number
    device_data: {
        bbl: number
        bbr: number
        bfl: number
        bfr: number
        tbl: number
        tbr: number
        tfl: number
        tfr: number
        o: number
        d: boolean
    }
}


// Keeping these streams in a different file than the component will prevent the file from being reloaded
// when the component file is saved after being edited
// That way, the stream content will not be reset on save
const [constantTelemetry$, setConstantTelemetry] = createSignal<TempDataEntry>()

interval(1000).pipe(
    combineLatestWith(currentDeviceId$),
    map(([_, deviceId]) => deviceId),
    filter(deviceId => deviceId !== null),
).subscribe((deviceId) => {
    if (deviceId) {
        const url = new URL(TELEMETRY_URL)

        url.searchParams.append("TableName", "temp_data")
        url.searchParams.append("Limit", "1")
        url.searchParams.append("device_id", deviceId)
        url.searchParams.append("ascending", "false")

        fetch(url.href)
            .then(res => res.json())
            .then(data => data.Items[0])
            .then(val => setConstantTelemetry(val))
            .catch(console.error)
    }
})

export const telemetry$ = constantTelemetry$.pipe(
    filter(val => !!val),
    distinct(entry => entry.sample_time)
)

export const [useTelemetry] = bind(telemetry$, null)
