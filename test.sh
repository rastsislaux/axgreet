#! /usr/bin/env bash
set -e

make build

ln -s /home/rastsislau/git/axgreet/main /tmp/axgreet
chmod +x /tmp/axgreet
chmod 777 /tmp/axgreet

sudo greetd --vt 2 --config /home/rastsislau/git/axgreet/greetd.toml

rm /tmp/axgreet
