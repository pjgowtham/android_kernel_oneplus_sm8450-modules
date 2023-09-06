# SPDX-License-Identifier: GPL-2.0-only

VIDEO_SELECT := CONFIG_MSM_VIDC_V4L2=m

KBUILD_OPTIONS += VIDEO_ROOT=$(KERNEL_SRC)/$(M)
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(VIDEO_SELECT)

KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(OUT_DIR)/vendor/qcom/opensource/mmrm-driver/Module.symvers

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(M) modules_install

%:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) $@ $(KBUILD_OPTIONS)

clean:
	rm -f *.o *.ko *.mod.c *.mod.o *~ .*.cmd Module.symvers
	rm -rf .tmp_versions
