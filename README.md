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
openssl x509 -req -in server.req -CA ca.pem -CAkey privkey.pem -CAserial file.srl \
    -out server.pem

# client cert
# make sure the CN contains string "control.freeshell.ustc.edu.cn"
openssl genrsa -out client.key
openssl req -key client.key -new -out client.req
openssl x509 -req -in client.req -CA ca.pem -CAkey privkey.pem -CAserial file.srl \
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

URL           | GET           | POST          | PUT           | DELETE        |
------------- | ------------- | ------------- | ------------- | ------------- |
/v1/vz        | ☑list vz ids  |               |               |               |
/v1/vz/\<id\> | ☑get info     | ☐create       | ☐set parameter| ☐delete       |
/v1/control/\<id\> |          | ☐control a vz |               |               |

* control: start,stop,reboot,reinstall