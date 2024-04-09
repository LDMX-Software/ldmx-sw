###############################################################################
# This dockerfile is meant to build the production image of ldmx-sw
#   for the development image, look at the LDMX-Software/docker repo
###############################################################################

FROM ldmx/dev:4.2.1

# install ldmx-sw into the container at /usr/local
COPY . /code
RUN mkdir /code/build &&\
    /etc/entry.sh /code/build cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. &&\
    /etc/entry.sh /code/build make install &&\
    rm -rf code &&\
    ldconfig /usr/local/lib

ENV LDMX_SW_INSTALL=/usr/local
