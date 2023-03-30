FROM ubuntu:20.04
RUN apt-get update && apt-get install -y llvm-dev git gcc g++ make findutils gawk
RUN useradd -ms /bin/bash xl-user
COPY . /xl
WORKDIR /xl
RUN make
RUN cp src/*.syntax .
RUN cp src/*.stylesheet .
RUN cp src/builtins.xl .
RUN ln -s /xl/xl /usr/local/bin
RUN ln -s /xl/libxl* /usr/local/lib
RUN ln -s /xl/librecorder* /usr/local/lib
USER xl-user
ENTRYPOINT ["xl"]
CMD ["--help"]
