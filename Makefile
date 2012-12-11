# This is top level makefile. The real shit is at src/Makefile

default:all

.DEFAULT:
	@cd ./channels && $(MAKE) $@
	@cd ./src && $(MAKE) $@

.PHONY:
	default
