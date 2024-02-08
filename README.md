# Smart Guitar Amplifier

TODO: Add Project Description

## Getting started

Before getting started, make sure you have a proper nRF Connect SDK development environment.
Follow the official
[Installation guide](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html).

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``smart-guitar-amp-fw`` and all nRF Connect SDK modules will be cloned. Run the following
command:

```shell
# initialize my-workspace for the smart-guitar-amp-fw (main branch)
west init -m https://github.com/maanika/smart-guitar-amp-fw.git --mr main my-workspace
# update nRF Connect SDK modules
cd my-workspace
west update
```

### Building and running

To build the application, run the following command:

```shell
west build -p -b raytac_mdbt50q_db_20_nrf52820 app -DBOARD_DIR=boards/arm/raytac_mdbt50q_db_20_nrf52820
```

Once you have built the application, run the following command to flash it:

```shell
west flash
```
