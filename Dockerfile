FROM debian:bookworm-slim

RUN \
  apt-get update && \
  apt-get upgrade -y && \
  apt-get install -y --no-install-recommends meson libsigrokcxx-dev libglibmm-2.4-dev libsdl2-dev libcxxopts-dev g++

WORKDIR /project
