# Docs: https://hub.docker.com/r/espressif/idf
# :latest may download the latest dev branch
# :release-v6.0 is only a preview version as of Nov 2025 (weird naming)
# There is no reason to upgrade this project to v6.0 (breaking changes: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html)
# The current latest stable version is 5.5, it has maintanance updates until 21 Jan 2028
# :release-v5.5 is tracking the latest changes in the 5.5 release branch, but I am using a specific version below to avoid unexpected breakages
# Versioning: https://docs.espressif.com/projects/esp-idf/en/release-v6.0/esp32/versions.html

FROM espressif/idf:v5.5.1

ARG DEBIAN_FRONTEND=nointeractive
ARG CONTAINER_USER=esp
ARG USER_UID=1050
ARG USER_GID=$USER_UID

# xclip is required by the api explorer script on linux
# libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0 are required by QEMU
RUN apt-get update \
  && apt install -y -q \
  cmake \
  gh \
  git \
  xclip \
  libgcrypt20 \
  libglib2.0-0 \
  libnuma1 \
  libpixman-1-0 \
  libsdl2-2.0-0 \
  libslirp0 \
  && rm -rf /var/lib/apt/lists/*

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV TZ="Europe/Budapest"
RUN date

RUN groupadd --gid ${USER_GID} ${CONTAINER_USER} \
  && adduser --uid ${USER_UID} --gid ${USER_GID} --disabled-password --gecos "" ${CONTAINER_USER} \
  && usermod -a -G root ${CONTAINER_USER} && usermod -a -G dialout ${CONTAINER_USER}

RUN chmod -R 775 /opt/esp/python_env/

# Switch to non-root user here
USER ${CONTAINER_USER}
ENV USER=${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}

# Put the IDF setup script into the bashrc so that it's available in all terminal sessions
RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

# Set default shell
CMD ["/bin/bash", "-c"]

# Install QEMU for ESP32 emulation
RUN python ${IDF_PATH}/tools/idf_tools.py install qemu-xtensa qemu-riscv32

# Install python packages needed for the api explorer
# The python in esp/python_env/python and python3 and python3.12 is a link to the /usr/bin/python
# The export script was complaining about IDF_PATH not being set when it is (it is set in the base container). Setting IDF_PATH_FORCE to 1 to bypass that check.
RUN export IDF_PATH_FORCE=1 && . /opt/esp/idf/export.sh && python -m pip install textual pyperclip

ENV PYTHONPATH="/workspaces/BarnaNet-LedController/components/BarnaNetLibrary/api:${PYTHONPATH}"
ENV PYTHONPATH="/workspaces/BarnaNet-LedController/api:${PYTHONPATH}"

# Add remote port to env
# Changing it in bash: export ESPPORT="rfc2217://host.docker.internal:4000?ign_set_control"
ENV ESPPORT="rfc2217://host.docker.internal:4000?ign_set_control"

# Configure git to avoid unsafe repository errors in devcontainer
RUN git config --global --add safe.directory /workspaces/BarnaNet-LedController/components/BarnaNetLibrary
RUN git config --global --add safe.directory /workspaces/BarnaNet-LedController

# Configure git to handle line endings correctly
RUN git config --global core.autocrlf true
