package=argon2
$(package)_version=20190702
$(package)_download_path=https://github.com/P-H-C/phc-winner-argon2/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=daf972a89577f8772602bf2eb38b6a3dd3d922bf5724d45e7f9589b5e830442c
$(package)_dependencies=

define $(package)_build_cmds
  $(MAKE) -C $($(package)_build_subdir) PREFIX=$(host_prefix) CC="$($(package)_cc)" AR="$($(package)_ar)"
endef

define $(package)_stage_cmds
  $(MAKE) -C $($(package)_build_subdir) install PREFIX=$($(package)_staging_prefix_dir)$(host_prefix)
endef
