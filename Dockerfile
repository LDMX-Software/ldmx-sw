###############################################################################
# This dockerfile is meant to build the production image of ldmx-sw
#   for the development image, look at the LDMX-Software/docker repo
###############################################################################

ARG DEV_TAG=v1.7-ldmx-sw
FROM ldmx/dev:${DEV_TAG}
# we need xgboost for the older BDT runner
RUN export PYTHONPATH=$ROOTSYS/lib &&\
    export CLING_STANDARD_PCH=none &&\
    export LD_LIBRARY_PATH=$XercesC_DIR/lib:$ROOTSYS/lib:$G4DIR/lib:$LD_LIBRARY_PATH &&\
    python -m pip install --upgrade --no-cache-dir \
        numpy \
        xgboost

# install ldmx-sw into the container at /usr/local
COPY . /code
RUN mkdir /code/build &&\
    ./home/ldmx.sh /code/build cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. &&\
    ./home/ldmx.sh /code/build make install

COPY ./scripts/docker_entrypoint.sh /home/docker_entrypoint.sh
RUN chmod 755 /home/docker_entrypoint.sh
ENTRYPOINT ["/home/docker_entrypoint.sh"]
