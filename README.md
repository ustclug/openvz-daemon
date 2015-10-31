# openvz-daemon

## Dependency

* [GNU Libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)
* [GnuTLS](http://www.gnutls.org/)
* [json-c](https://github.com/json-c/json-c/wiki)

## Build & Run

```bash
# build
mkdir build
cd build
cmake ..
make
```

```bash
# generate SSL cert
# CA
openssl req -out ca.pem -new -x509
echo "00" > file.srl
# server cert
openssl genrsa -out server.key
openssl req -key server.key -new -out server.req
openssl x509 -req -in server.req -CA CA.pem -CAkey privkey.pem -CAserial file.srl \
    -out server.pem

# client cert
# make sure the CN contains string "control.freeshell.ustc.edu.cn"
openssl genrsa -out client.key
openssl req -key client.key -new -out client.req
openssl x509 -req -in client.req -CA CA.pem -CAkey privkey.pem -CAserial file.srl \
    -out client.pem

```

```bash
# server start
./openvz_daemon

# client test
curl --cacert ca.pem --cert client.pem --key client.key https://localhost:8888/

```

## API

### Overview

#### shells
* [x] `GET` /v1/shells
    * @des: list all freeshell ids
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
* [ ] `POST` /v1/shells
    * @des: create a new freeshell and return its id
    * @para: `unimplemented`
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
* [ ] `GET` /v1/shells/{id}
    * @des: get all info about the freeshell with id {id}
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found
* [ ] `PUT` /v1/shells/{id}
    * @des: update the info about freeshell with id {id}
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found
* [ ] `DELETE` /v1/shells/{id}
    * @des: delete a freeshell
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found

#### control
* [ ] `PUT` /v1/control/{id}/start
    * @des: start
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found
* [ ] `PUT` /v1/control/{id}/stop
    * @des: stop
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found
* [ ] `PUT` /v1/control/{id}/reboot
    * @des: reboot
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found
* [ ] `PUT` /v1/control/{id}/reinstall
    * @des: reinstall
    * @para: `unimplemented`
    * @ret: 200 Success
    * @ret: 400 Bad Request
    * @ret: 403 Unauthorized access
    * @ret: 404 Not found

#### others
