FROM ubuntu:xenial

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get dist-upgrade -y \
 && apt-get install -y sudo \
 && rm -rf /var/lib/apt/lists/*

RUN locale-gen en_US.UTF-8 \
 && update-locale LANG=en_US.UTF-8 LANGUAGE=en_US:en
ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

RUN adduser --disabled-password --gecos '' docker \
 && adduser docker sudo \
 && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    autoconf \
    automake \
    autopoint \
    libtool \
    gettext \
    git \
    openssh-client \
    liblzo2-dev \
    zlib1g-dev \
    libpng-dev \
    libgif-dev \
    libjpeg-turbo8-dev \
    libexpat1-dev \
    libsqlite3-dev \
    libfreetype6-dev \
    libpcre++-dev \
    libtinyxml-dev \
    libpython2.7-dev \
    libyajl-dev \
    libfribidi-dev \
    libtag1-dev \
    uuid-dev \
    libmicrohttpd-dev \
    libssh-dev \
    libxml2-dev \
    libxslt1-dev \
    libasound2-dev \
    libudev-dev \
    libdbus-1-dev \
    libavahi-client-dev \
    libsmbclient-dev \
    libpulse-dev \
    libvdpau-dev \
    libva-dev \
    libcurl4-openssl-dev \
    libass-dev \
    libcec-dev \
    libbluray-dev \
    libplist-dev \
    libnfs-dev \
    libssl-dev \
    libiso9660-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libdrm-dev \
    libxrandr-dev \
    libegl1-mesa-dev \
    python2.7 \
    default-jre-headless \
    swig \
    yasm \
    --no-install-recommends \
 && rm -rf /var/lib/apt/lists/*

USER docker
