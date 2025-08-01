FROM --platform=$BUILDPLATFORM datadog/dd-trace-ci:buster AS src
ARG phpTarGzUrl
ARG phpSha256Hash
COPY 0001-Better-support-for-cross-compilation.patch /home/circleci
COPY 0001-Fix-GH-10611-fpm_env_init_main-leaks-environ.patch /home/circleci
COPY php-8.0/0001-Disable-ZEND_RC_MOD_CHECK-while-loading-shared-exten.patch /home/circleci
COPY php-8.0/0001-Fix-memory-leak-in-zend_wrong_callback_error.patch /home/circleci
COPY php-8.0/0001-Introduce-DD_IGNORE_ARGINFO_ZPP_CHECK-env-var-to-ski.patch /home/circleci
RUN set -eux; \
    curl -fsSL -o /tmp/php.tar.gz "${phpTarGzUrl}"; \
    (echo "${phpSha256Hash} /tmp/php.tar.gz" | sha256sum -c -); \
    tar xf /tmp/php.tar.gz -C "${PHP_SRC_DIR}" --strip-components=1; \
    rm -f /tmp/php.tar.gz; \
    cd ${PHP_SRC_DIR}; \
    git apply /home/circleci/0001-Better-support-for-cross-compilation.patch; \
    git apply /home/circleci/0001-Introduce-DD_IGNORE_ARGINFO_ZPP_CHECK-env-var-to-ski.patch; \
    git apply /home/circleci/0001-Fix-memory-leak-in-zend_wrong_callback_error.patch; \
    git apply /home/circleci/0001-Disable-ZEND_RC_MOD_CHECK-while-loading-shared-exten.patch; \
    patch -p1 /home/circleci/0001-Fix-GH-10611-fpm_env_init_main-leaks-environ.patch; \
    # We need to remove the dl() calls from run-tests as some extensions don't properly switch interned string storage for INIs. \
    sed -i 's/\bdl(/(bool)(/' run-tests.php; \
    ./buildconf --force

FROM --platform=$BUILDPLATFORM datadog/dd-trace-ci:buster AS build
ARG phpVersion
ENV PHP_VERSION=${phpVersion}
ENV PHP_INSTALL_DIR=${PHP_INSTALL_DIR}
ARG TARGETPLATFORM
ARG BUILDPLATFORM
ARG sharedBuild=0
ENV SHARED_BUILD=${sharedBuild}
# install cross-compiling packages
RUN if [ "$BUILDPLATFORM" != "$TARGETPLATFORM" ]; then \
        sudo dpkg --add-architecture ${TARGETPLATFORM#linux/}; \
        sudo apt update; \
        sudo apt-get install -y --no-install-recommends valgrind:${TARGETPLATFORM#linux/} $(echo $DEVLIBS | sed "s/\(\s\|$\)/:${TARGETPLATFORM#linux/}\1/g"); \
    fi
COPY build-php.sh /home/circleci
COPY --chown=circleci:circleci --from=src $PHP_SRC_DIR $PHP_SRC_DIR

FROM build AS php-debug
ARG TARGETPLATFORM
RUN /home/circleci/build-php.sh $TARGETPLATFORM $PHP_INSTALL_DIR debug $PHP_VERSION $SHARED_BUILD
COPY build-extensions.sh /home/circleci
RUN /home/circleci/build-extensions.sh

FROM build AS php-nts
ARG TARGETPLATFORM
RUN /home/circleci/build-php.sh $TARGETPLATFORM $PHP_INSTALL_DIR nts $PHP_VERSION $SHARED_BUILD
COPY build-extensions.sh /home/circleci
RUN /home/circleci/build-extensions.sh

FROM build AS php-zts
ARG TARGETPLATFORM
RUN /home/circleci/build-php.sh $TARGETPLATFORM $PHP_INSTALL_DIR zts $PHP_VERSION $SHARED_BUILD
COPY build-extensions.sh /home/circleci
RUN /home/circleci/build-extensions.sh

FROM build AS php-debug-zts-asan
ARG TARGETPLATFORM
RUN /home/circleci/build-php.sh $TARGETPLATFORM $PHP_INSTALL_DIR debug-zts-asan $PHP_VERSION $SHARED_BUILD \
    && mv /tmp/build-php/sapi/cli/php /tmp/build-php/sapi/cli/php-asan
# for reasons the asan build just ... crashes in qemu (or executed via bytecode interpreter). The binary itself however perfectly works on the target platform
# just put a working php binary there to continue installation and be happy, then copy back later.
COPY --chown=circleci:circleci --from=php-debug $PHP_INSTALL_DIR/debug/bin/php /tmp/build-php/sapi/cli/php
RUN set -eux; ln -s $PHP_INSTALL_DIR/debug-zts-asan $PHP_INSTALL_DIR/debug; cd /tmp/build-php; touch sapi/cli/php; make install; switch-php debug-zts-asan;
COPY build-extensions.sh /home/circleci
RUN /home/circleci/build-extensions.sh
RUN cp /tmp/build-php/sapi/cli/php-asan $PHP_INSTALL_DIR/debug-zts-asan/bin/php

FROM datadog/dd-trace-ci:buster AS final
COPY --chown=circleci:circleci --from=src $PHP_SRC_DIR $PHP_SRC_DIR
COPY --chown=circleci:circleci --from=php-debug-zts-asan $PHP_INSTALL_DIR/debug-zts-asan $PHP_INSTALL_DIR/debug-zts-asan
COPY --chown=circleci:circleci --from=php-debug $PHP_INSTALL_DIR/debug $PHP_INSTALL_DIR/debug
COPY --chown=circleci:circleci --from=php-nts $PHP_INSTALL_DIR/nts $PHP_INSTALL_DIR/nts
COPY --chown=circleci:circleci --from=php-zts $PHP_INSTALL_DIR/zts $PHP_INSTALL_DIR/zts

USER root

RUN set -eux; \
    # Enable the apache config \
    echo "LoadModule php_module /usr/lib/apache2/modules/libphp.so" | tee /etc/apache2/mods-enabled/php.load; \
    # Set the default PHP version
    switch-php debug;

# Install Composer
COPY --from=composer:1 /usr/bin/composer /usr/local/bin/composer

COPY welcome /etc/motd
RUN sudo sed -i "s/VERSIONS/$(ls $PHP_INSTALL_DIR | xargs)/" /etc/motd

USER circleci

CMD ["php-fpm", "-F"]
