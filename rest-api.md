# API

We design it according to [LXD REST API](https://github.com/lxc/lxd/blob/master/specs/rest-api.md)

## Return

### Standard

HTTP code is set to 200.

```json
{
    "type": "sync",
    "status": "success",
    "status_code": 200,
    "metadata": {}
}
```

### Background operation

HTTP code is set to 202.

Location HTTP header is set to the operation URL.

```json
{
    "type": "async",
    "status": "ok",
    "status_code": 100,
    "operation": "/1.0/vz/<id>",          # URL to the background operation
    "resources": {
        "containers": ["/1.0/vz/id"]      # List of affected resources
    },
    "metadata": {}                        # Metadata relevant to the operation
}
```

### Error

HTTP code is set to 400, 401, 403, 404, 409, 412 or 500.

```json
{
    "type": "error",
    "error": "failure",
    "error_code": 400,
    "metadata": {}                      # More details about the error
}
```

## Status Codes

### List of current status codes

Code  | Meaning
:---  | :------
100   | OK
101   | Started
102   | Stopped
103   | Running
104   | Cancelling
105   | Pending
106   | Starting
107   | Stopping
108   | Aborting
109   | Freezing
110   | Frozen
111   | Thawed
200   | Success
400   | Failure
401   | Cancelled

## Structure

* /1.0
    * [/1.0/vz](#10-vz)
        * [/1.0/vz/\<id\>](#10-vz-id)
            * [/1.0/vz/\<id\>/state](#10-vz-id-state)

## <a name="10-vz"></a> /1.0/vz

### GET

* Description: list of containers
* Authentication: trusted
* Operation: sync
* Return: list of vz containers' id.

Output:
```json
[101, 102]
```

## <a name="10-vz-id"></a> /1.0/vz/\<id\>

### GET
* Description: container information
* Authentication: trusted
* Operation: sync
* Return: dict of the container configuration and current state.

Output:
```json
{
    "hostname": "my-container",
    "config": {
        "limits.cpus": "3",
        "disk_space": "5GB",
        "memory": "4GB",
        "ip_address": "10.xx.x.xxx"
    },
    "status": {
        "status": "Running",
        "status_code": 103,
        "disk_usage": "3GB",
        "free_memory": "2GB",
        "cpu_load": "20%",
    }
}
```

### POST
* Description: Create a new container
* Authentication: trusted
* Operation: async
* Return: background operation or standard error

Input:
```json
{
    "hostname": "my-new-container",                 # 64 chars max, ASCII, no slash, no colon and no comma
    "config": {
        "limits.cpus": "2"                          # Config override.
    },
    "source": {
        "type": "image",                            # Can be: "image", "migration", "copy" or "none"
        "properties": {                             # Properties
            "os": "ubuntu",
            "release": "14.04",
            "architecture": "x86_64"
        }
    }
}
```

### PUT
* Description: update container configuration or restore snapshot
* Authentication: trusted
* Operation: async
* Return: background operation or standard error

Input (update container configuration):

Takes the same structure as that returned by GET but doesn't allow name changes (see POST below)
or changes to the status sub-dict (since that's read-only).

### DELETE
* Description: remove the container
* Authentication: trusted
* Operation: async
* Return: background operation or standard error

Input (none at present)

## <a name="10-vz-id-state"></a> /1.0/vz/\<id\>/state

### GET

* Description: current state
* Authentication: trusted
* Operation: sync
* Return: dict representing current state

Output:
```json
{ "status": "Running", "status_code": 103  }
```

### PUT

* Description: change the container state
* Authentication: trusted
* Operation: async
* Return: background operation or standard error

Input:
```json
{
    "action": "stop",       # State change action (stop, start, restart, freeze or unfreeze)
    "timeout": 30,          # A timeout after which the state change is considered as failed
    "force": true           # Force the state change (currently only valid for stop and restart where it means killing the container)

}
```
