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

## Structure

* /1.0
    * [/1.0/vz](#10-vz)
        * [/1.0/vz/\<id\>](#10-vz-id)
            * [/1.0/vz/\<id\>/state](#10-vz-id-state)

## <a name="10-vz"></a> /1.0/vz

### GET


## <a name="10-vz-id"></a> /1.0/vz/\<id\>

### GET
* Description: Container information
* Authentication: trusted
* Operation: sync
* Return: dict of the container configuration and current state.

Output:
```json
{
    "hostname": "my-container",
    "config": {"limits.cpus": "3"},
    "status": {
                "status": "Running",
                "status_code": 103,
                "machine": [{
                    "disk_space": "5GB",
                    "disk_usage": "3GB",
                    "memory": "4GB",
                    "free_memory": "2GB",
                    "cpu_load": "20%",
                    "ip_address": "10.xx.x.xxx"
                }]
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
    "hostname": "my-new-container",                                         # 64 chars max, ASCII, no slash, no colon and no comma
    "config": {"limits.cpus": "2"},                                     # Config override.
    "source": {"type": "image",                                         # Can be: "image", "migration", "copy" or "none"
               "properties": {                                          # Properties
                    "os": "ubuntu",
                    "release": "14.04",
                    "architecture": "x86_64"
                }}
}
```

### PUT
* Description: update container configuration or restore snapshot
* Authentication: trusted
* Operation: async
* Return: background operation or standard error

Input (update container configuration):

Takes the same structure as that returned by GET but doesn't allow name changes (see POST below) or changes to the status sub-dict (since that"s read-only).

### DELETE
* Description: remove the container
* Authentication: trusted
* Operation: async
* Return: background operation or standard error

Input (none at present):

```json
{
}
```

HTTP code for this should be 202 (Accepted).

## <a name="10-vz-id-state"></a> /1.0/vz/\<id\>/state

### GET

### PUT
