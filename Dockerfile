###############################################################################
# This dockerfile is meant to build the production image of ldmx-sw
#   for the development image, look at the LDMX-Software/dev-build-context repo
###############################################################################

FROM ldmx/dev:v5.2.2

# install ldmx-sw into the container at /usr/local
COPY . /code
RUN cmake -DCMAKE_INSTALL_PREFIX=/usr/local -S /code -B /code/build &&\
    cmake --build /code/build --target install &&\
    rm -rf /code &&\
    ldconfig

ENV LDMX_SW_INSTALL=/usr/local
