load("//build/kernel/kleaf:kernel.bzl", "ddk_headers")
load("//build/kernel/oplus:oplus_modules_define.bzl", "define_oplus_ddk_module", "oplus_ddk_get_kernel_version", "oplus_ddk_get_target", "oplus_ddk_get_variant", "bazel_support_platform")
load("//build/kernel/oplus:oplus_modules_dist.bzl", "ddk_copy_to_dist_dir")

def define_oplus_local_modules():
    target = oplus_ddk_get_target()
    variant  = oplus_ddk_get_variant()
    kernel_build_variant = "{}_{}".format(target, variant)
    kernel_version = oplus_ddk_get_kernel_version()

    if bazel_support_platform == "qcom" :
        haptic_feedback_ko_deps = [
            "//vendor/oplus/kernel/dft/bazel:oplus_bsp_dft_kernel_fb",
        ]
        haptic_ko_deps = [
            "//vendor/oplus/kernel/vibrator/bazel:oplus_bsp_haptic_feedback",
            "//vendor/oplus/kernel/boot:oplus_bsp_boot_projectinfo",
            "//vendor/oplus/kernel/boot:oplus_bsp_bootmode",
        ]
        haptic_copts = []
    else :
        haptic_feedback_ko_deps = [
            "//kernel_device_modules-{}/drivers/soc/oplus/dft/bazel:oplus_bsp_dft_kernel_fb".format(kernel_version),
        ]
        haptic_ko_deps = [
            "//kernel_device_modules-{}/drivers/misc/oplus/vibrator/bazel:oplus_bsp_haptic_feedback".format(kernel_version),
            "//kernel_device_modules-{}/drivers/soc/oplus/boot:oplus_bsp_boot_projectinfo".format(kernel_version),
            "//kernel_device_modules-{}/drivers/misc/mediatek/boot_common:mtk_boot_common".format(kernel_version),
            "//kernel_device_modules-{}/drivers/soc/oplus/boot:oplusboot".format(kernel_version),
        ]
        haptic_copts = [
            "-I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/include/",
        ]

    define_oplus_ddk_module(
        name = "oplus_bsp_haptic_feedback",
        srcs = native.glob([
            "haptic_feedback/*.h",
            "haptic_feedback/haptic_feedback.c",
        ]),
        hdrs = ["haptic_feedback/haptic_feedback.h"],
        includes = ["."],
        ko_deps = haptic_feedback_ko_deps,
        local_defines = ["CONFIG_OPLUS_FEATURE_FEEDBACK"],
    )

    define_oplus_ddk_module(
        name = "oplus_bsp_haptic",
        srcs = native.glob([
            "haptic_feedback/*.h",
            "oplus_haptic/**/*.h",
            "oplus_haptic/aw_haptic/*.c",
            "oplus_haptic/haptic_common/*.c",
            "oplus_haptic/si_haptic/*.c",
        ]),
        copts = haptic_copts,
        ko_deps = haptic_ko_deps,
        includes = ["."],
        local_defines = [
            "CONFIG_AW_HAPTIC",
            "CONFIG_SIH_HAPTIC",
            "CONFIG_OPLUS_HAPTIC_COMMON",
            "CONFIG_HAPTIC_FEEDBACK_MODULE",
        ],
        conditional_defines = {
            "mtk":  ["CONFIG_OPLUS_CHARGER_MTK", "OPLUS_FEATURE_CHG_BASIC"],
            "qcom":  ["OPLUS_FEATURE_CHG_BASIC"],
        },
    )

    ddk_copy_to_dist_dir(
        name = "oplus_haptic",
        module_list = [
            "oplus_bsp_haptic_feedback",
            "oplus_bsp_haptic",
        ],
    )
