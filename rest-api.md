# API

We design it according to [LXD REST API](https://github.com/lxc/lxd/blob/master/specs/rest-api.md)

## Return

### Standard

HTTP code is set to 200.

```json
{
    'type': "sync",
    'status': "success",
    'status_code': 200,
    'metadata': {}
}
```

### Background operation

HTTP code is set to 202.

Location HTTP header is set to the operation URL.

```json
{
    'type': "async",
    'status': "ok",
    'status_code': 100,
    'operation': "/1.0/vz/<id>",          # URL to the background operation
    'resources': {
        'containers': ["/1.0/vz/id"]      # List of affected resources
    },
    'metadata': {}                        # Metadata relevant to the operation
}
```

### Error

HTTP code is set to 400, 401, 403, 404, 409, 412 or 500.

```json
{
    'type': "error",
    'error': "failure",
    'error_code': 400,
    'metadata': {}                      # More details about the error
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

### POST

### PUT

### DELETE


## <a name="10-vz-id-state"></a> /1.0/vz/\<id\>/state

### GET

### PUT
