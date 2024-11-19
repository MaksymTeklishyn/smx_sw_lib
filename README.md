
# SMX Detector and Electronics Handling Libraries

This project provides a set of generic C++ libraries to handle SMX (STS-MUCH-XYTER) ASIC-based detectors and associated electronics. These libraries support various objects related to the detector setup, including ASICs, Front-End Boards (FEBs), Modules, micro-cables, and sensors, as well as the operations and results for tasks such as p-scan and calibration/trim. Built as a collection of state machines, these classes enable structured handling of detector-related objects and processes.

## Downloading the Libraries

To download the project from a Git repository, use the following command in your terminal:

```bash
git clone https://github.com/MaksymTeklishyn/smx_sw_lib
```

## Generating Documentation with Doxygen

To generate the documentation on your PC using Doxygen, follow these steps:

1. Install Doxygen if you haven’t already. You can install it using your package manager or download it from [Doxygen's official site](https://www.doxygen.nl/download.html).

2. Once installed, navigate to the project directory and generate the Doxygen documentation:

   ```bash
   doxygen Doxyfile
   ```

   This will generate HTML and LaTeX documentation (if configured in the `Doxyfile`), which you can view locally in the `html` directory within the output path specified in `Doxyfile`.

3. Open the Doxygen documentation:

   ```bash
   firefox docs/html/index.html
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
   ./read_pscan data/*.txt
   ```

   This command reads the `.txt` file, parses its contents, and outputs the processed data to a ROOT file in the specified output location.

To access the `pscanTree` in your `.root` files from the command line or within a ROOT session, you can follow these steps:

To access the `pscanTree` using the new `TBrowser` in ROOT, follow these steps:

## Using `TBrowser` to Explore `pscanTree`

1. **Start ROOT:**
   Open a terminal and start a ROOT session with your `.root` files:

   ```bash
   root data/*.root
   ```

   This command will open ROOT and load the first matching `.root` file in the `data` directory.

2. **Open `TBrowser`:**
   Launch the interactive `TBrowser` GUI by typing:

   ```cpp
   new TBrowser();
   ```

## Alternative Scripting in the Same Session

If you are already in a ROOT session and want to work interactively after exploring the file in `TBrowser`, you can still retrieve and manipulate the `pscanTree` using:

```cpp
TTree* tree = (TTree*)gDirectory->Get("pscanTree");
tree->Print(); // View the structure of the tree
tree->Scan();  // Print its data
```

This workflow combines the graphical capabilities of `TBrowser` with the power of scripting, enabling you to explore, visualize, and process your data seamlessly.
