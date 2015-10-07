# openvz-daemon

## Dependency

* [GNU Libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)
* [GnuTLS](http://www.gnutls.org/)
* [json-c](https://github.com/json-c/json-c/wiki)

## API

### Overview

URL           | GET           | POST          | PUT           | DELETE        |
------------- | ------------- | ------------- | ------------- | ------------- |
/vz           | list vz ids   |               |               |               |
/vz/\<id\>    | get info      | create        | set parameter | delete        |
/control/\<id\> |             | control a vz  |               |               |

* control: start,stop,reboot,reinstall