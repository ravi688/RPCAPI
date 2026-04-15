# RPCAPIClient for python
By using this python module, an application can communicate and make API requests to the server implementing its RPC API using RPCAPIServer

## Setting up python virtual environment
```
mkdir .python_env
python -m venv .python_env
source .python_env/bin/activate
```
## Installing the python package
```
git clone https://github.com/ravi688/RPCAPI.git
cd RPCAPI/pyRPCAPIClient/package/
pip install .
```
## Running the python test client
```
cd RPCAPI/pyRPCAPIClient/test_scripts
python api_request.py
```
## Building and Running the C++ test server
```
cd RPCAPI
build_master meson setup build
build_master meson compile -C build
./build/TestRPCAPIServer
```

