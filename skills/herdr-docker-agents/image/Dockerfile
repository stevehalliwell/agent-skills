# Build: docker build -t pi-herdr-test:1.1.0 <skill-dir>/image
# Credentials and agent state belong to runtime containers, never this build.
FROM node:24-bookworm-slim@sha256:d6aa754f16b3197301076f047b5def2f02ea1dbbc2ca920407d46d7ec7f87b20

RUN apt-get update \
    && apt-get install -y --no-install-recommends bash ca-certificates fd-find git ripgrep \
    && ln -s /usr/bin/fdfind /usr/local/bin/fd \
    && rm -rf /var/lib/apt/lists/* \
    && npm install -g --ignore-scripts @earendil-works/pi-coding-agent@1.1.0 \
    && npm cache clean --force \
    && mkdir -p /workspace /home/node/.pi/agent \
    && chown -R node:node /workspace /home/node/.pi

USER node
WORKDIR /workspace
ENV PI_CODING_AGENT_DIR=/home/node/.pi/agent
ENTRYPOINT ["pi"]
