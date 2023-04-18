APP_TAR := userspace

# Specify directory of user application here
APPLICATIONS :=		\
	helloWorld

# Verify that user application's elf is name 'app.elf'
all:
	tar -cf $(APP_TAR).tar --files-from /dev/null
	for app in $(APPLICATIONS); do		\
		$(MAKE) -C $$app;				\
		tar --append --file=$(APP_TAR).tar -C $$app/bin/ $$app.elf;	\
	done

clean:
	rm -f $(APP_TAR).tar
	for app in $(APPLICATIONS); do  \
		$(MAKE) -C $$app clean; \
	done
