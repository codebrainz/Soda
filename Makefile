all:
	$(MAKE) -C Soda $@

clean:
	$(MAKE) -C Soda $@

test:
	$(MAKE) -C Soda test

.PHONY: all clean
