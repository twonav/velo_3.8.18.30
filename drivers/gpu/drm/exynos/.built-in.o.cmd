cmd_drivers/gpu/drm/exynos/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/gpu/drm/exynos/built-in.o drivers/gpu/drm/exynos/exynosdrm.o ; scripts/mod/modpost drivers/gpu/drm/exynos/built-in.o