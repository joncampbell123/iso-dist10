all:
	make -C dist10

clean:
	make -C dist10 clean
	make -C mathgen clean

distclean:
	make -C dist10 distclean
	make -C mathgen distclean

