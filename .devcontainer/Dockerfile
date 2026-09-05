# Use Miniforge as the base image for fast Conda package resolution
FROM condaforge/miniforge3:latest

# Prevent tzdata and apt prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install standard system utilities and debuggers
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    gdb \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Create a Conda environment and install our C++ dependencies.
# We include 'compilers' to get conda-forge's gcc/g++ which ensures ABI compatibility
# with the pre-compiled libraries.
RUN mamba create -n dev -c conda-forge -y \
    cmake \
    ninja \
    pkg-config \
    compilers \
    pinocchio \
    coal \
    protobuf \
    libprotobuf \
    libspatialindex \
    && mamba clean -afy

# Setup bash to automatically activate the 'dev' environment upon container entry
RUN echo "source /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc && \
    echo "conda activate dev" >> ~/.bashrc

# Set environment variables so CMake and compilers default to the Conda environment
ENV CONDA_DEFAULT_ENV=dev
ENV PATH=/opt/conda/envs/dev/bin:$PATH
# This is critical: it tells CMake exactly where to find the config files for Pinocchio/Coal
ENV CMAKE_PREFIX_PATH=/opt/conda/envs/dev

# Set the working directory to mount your code into
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]