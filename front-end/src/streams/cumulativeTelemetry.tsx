import { bind } from "@react-rxjs/core"
import { map, scan } from "rxjs"
import { telemetry$, TempDataEntry } from "./telemetry"

type ReadableTempDataEntry = TempDataEntry & {
    BottomBackLeft: number
    BottomBackRight: number
    BottomFrontLeft: number
    BottomFrontRight: number
    TopBackLeft: number
    TopBackRight: number
    TopFrontLeft: number
    TopFrontRight: number
    Outside: number
    DoorOpen: boolean
    Time: string
}

// Keeping these streams in a different file than the component will prevent the file from being reloaded
// when the component file is saved after being edited
// That way, the stream content will not be reset on save
export const cumulativeTelemetry$ = telemetry$.pipe(
    map((telemetry: TempDataEntry) => ({
        BottomBackLeft: Math.round(telemetry.device_data.bbl),
        BottomBackRight: Math.round(telemetry.device_data.bbr),
        BottomFrontLeft: Math.round(telemetry.device_data.bfl),
        BottomFrontRight: Math.round(telemetry.device_data.bfr),
        TopBackLeft: Math.round(telemetry.device_data.tbl),
        TopBackRight: Math.round(telemetry.device_data.tbr),
        TopFrontLeft: Math.round(telemetry.device_data.tfl),
        TopFrontRight: Math.round(telemetry.device_data.tfr),
        Outside: Math.round(telemetry.device_data.o),
        DoorOpen: telemetry.device_data.d,
        Time: new Date(telemetry.sample_time).toLocaleTimeString(),
    } as ReadableTempDataEntry)),
    scan((all, current) => current ? [...all, current] : all, [] as ReadableTempDataEntry[])
)

export const [useCumulativeTelemetry] = bind(cumulativeTelemetry$, [])
