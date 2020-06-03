###############################################################################
# This dockerfile is meant to build the production image of ldmx-sw
#   for the development image, look at the LDMX-Software/docker repo
###############################################################################

FROM ldmx/dev:latest
COPY . /code
RUN /bin/bash /code/scripts/docker_install.sh

COPY ./scripts/docker_entrypoint.sh /home/docker_entrypoint.sh
RUN chmod 755 /home/docker_entrypoint.sh
ENTRYPOINT ["/home/docker_entrypoint.sh"]
