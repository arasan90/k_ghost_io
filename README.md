# k_ghost_io

k_ghost_io is a C library designed to provide advanced HTTP/REST API and Server-Sent Events (SSE) functionalities, with a special focus on the simulation of hardware devices and complex modules (such as anemometers). This makes it ideal for development and testing environments where real hardware is not available or practical to use.

The library enables real-time communication between clients and simulated hardware through HTTP REST endpoints and SSE streams.

The project is structured to facilitate development, testing, and integration with other libraries.

## Features

- **Hardware Simulation**: Simulate complex hardware devices and modules via HTTP REST API
- **Real-time Communication**: Server-Sent Events (SSE) for real-time data streaming
- **Extensible Architecture**: Easily add new device types and I/O operations through interface registration
- **Mock Support**: Complete mock implementations for testing (using FFF framework)
- **Cross-platform**: Designed to work across different platforms, currently only Linux platform is available
- **Test Coverage**: Comprehensive unit tests with Google Test

## Project Structure

- **src/**: Contains the main source files of the library
- **include/**: Contains the public headers of the library, to be included in projects using k_ghost_io
- **mock/**: Contains library mocks, useful for testing other libraries that depend on k_ghost_io
- **test/**: Contains unit and integration tests, mainly written with Google Test
- **libs/**: External dependencies and libraries

## Requirements

- CMake >= 3.10
- A compatible C compiler (GCC, Clang, etc.)

## Build

1. Clone the repository:

   ```sh
   git clone https://github.com/arasan90/k_ghost_io.git
   cd k_ghost_io
   ```

2. Initialize and update git submodules (required for cjson dependency):

   ```sh
   git submodule update --init
   ```

3. Create a build directory and compile:
   ```sh
   mkdir build && cd build
   cmake .. -DK_GHOST_IO_DEV=ON
   make
   ```

## Running Tests

After building, you can run the unit tests:

```sh
ctest --output-on-failure
```

Or run the test executable directly:

```sh
./test/k_ghost_io_test
```

## Usage

To use the library in your project:

1. Include the headers from `include/` in your source code
2. Link the generated static library (`libk_ghost_io.a`)
3. Use the provided `k_ghost_io.cmake` file for easy CMake integration
4. Use the provided API functions for I/O operations

The library provides a convenient `k_ghost_io.cmake` file that contains helper functions to easily integrate k_ghost_io into your CMake-based projects.

Example:

```c
#include "k_ghost_io.h"

// Example callback function for handling interface requests
int my_device_callback(const cJSON *input_data_p) {
    // Process the input data for your simulated device
    printf("Received data for device simulation\n");
    return 0; // Return 0 on success, -1 on failure
}

// Example sync callback for SSE events
void my_sync_callback(void) {
    // Send status updates via SSE
    k_ghost_io_send_event("{\"status\":\"device_active\",\"value\":42}");
}

int main() {
    // Initialize the library
    if (k_ghost_io_init() != 0) {
        printf("Failed to initialize k_ghost_io\n");
        return -1;
    }

    // Register an interface for your simulated device
    k_ghost_io_register_interface("anemometer", my_device_callback, my_sync_callback);

    // The library will handle HTTP requests and SSE connections
    // Your application can continue with other tasks

    return 0;
}
```

## Mock and Hardware Simulation

The `mock/` folder contains mock implementations of the library functions using the FFF (Fake Function Framework), allowing you to:

- Test other libraries or applications that depend on k_ghost_io

The mock library (`libk_ghost_io_mock.a`) can be linked instead of the main library for testing purposes. The library supports:

- **REST API endpoints**: `/api/simulate` for device control and data input
- **SSE endpoint**: `/api/sse` for real-time status updates and data streaming
- **Interface registration**: Register custom callbacks for different device types
- **Real-time events**: Send data to connected clients via Server-Sent Events

**Note**: The server port (default: 8080) and API endpoints can be modified by defining the appropriate macros during compilation:

- `K_GHOST_IO_SERVER_PORT` - Changes the default server port
- `K_GHOST_IO_SSE_URI_PATH` - Changes the SSE endpoint path (default: `/api/sse`)
- `K_GHOST_IO_REST_URI_PATH` - Changes the REST API endpoint path (default: `/api/simulate`)

## Development

This project uses CMake for building and supports development mode with additional features:

- Code coverage analysis
- Strict compiler warnings and error checking
- Automatic dependency management
- Integrated testing framework

## Contributing

Contributions via pull request are welcome. Please:

1. Add tests for every new feature or bugfix
2. Follow the existing code style
3. Ensure all tests pass before submitting
4. Update documentation as needed

## License

This project is distributed under the GNU General Public License v3.0. See the `LICENSE` file for details.

## Contact

For questions or bug reports, open an issue on GitHub.
