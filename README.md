
# SMX Detector and Electronics Handling Libraries

This project provides a set of generic C++ libraries to handle SMX (STS-MUCH-XYTER) ASIC-based detectors and associated electronics. These libraries support various objects related to the detector setup, including ASICs, Front-End Boards (FEBs), Modules, micro-cables, and sensors, as well as the operations and results for tasks such as p-scan and calibration/trim. Built as a collection of state machines, these classes enable structured handling of detector-related objects and processes.

## Downloading the Libraries

To download the project from a Git repository, use the following command in your terminal:

```bash
git clone <repository_url>
```

Replace `<repository_url>` with the actual URL of the Git repository.

## Generating Documentation with Doxygen

To generate the documentation on your PC using Doxygen, follow these steps:

1. Install Doxygen if you haven’t already. You can install it using your package manager or download it from [Doxygen's official site](https://www.doxygen.nl/download.html).

2. Once installed, navigate to the project directory and generate the Doxygen documentation:

   ```bash
   doxygen Doxyfile
   ```

   This will generate HTML and LaTeX documentation (if configured in the `Doxyfile`), which you can view locally in the `html` directory within the output path specified in `Doxyfile`.

## Running the Example: `read_pscan`

The example program `read_pscan` demonstrates reading a p-scan `.txt` file, parsing its data, and writing the results to a ROOT file.

1. Build the project using the provided `Makefile`:

   ```bash
   make
   ```

   This will generate the `read_pscan` executable.

2. Run `read_pscan` with a specified p-scan `.txt` file as follows:

   ```bash
   ./read_pscan path/to/your/pscan_file.txt
   ```

   This command reads the `.txt` file, parses its contents, and outputs the processed data to a ROOT file in the specified output location.

