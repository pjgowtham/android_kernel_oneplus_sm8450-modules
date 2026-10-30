KBUILD_OPTIONS+= EVA_ROOT=$(KERNEL_SRC)/$(M)
KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS+=$(M)/../dsp-kernel/Module.symvers
KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS+=$(M)/../mmrm-driver/Module.symvers
KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS+=$(M)/../synx-kernel-v1/Module.symvers

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) M=$(M) -C $(KERNEL_SRC) modules_install

%:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) $@ $(KBUILD_OPTIONS)

clean:
	rm -f *.o *.ko *.mod.c *.mod.o *~ .*.cmd Module.symvers
	rm -rf .tmp_versions
