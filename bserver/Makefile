
subdirs = common/src \
	  server/src

targets = all clean 

$(targets):
	@for dir in $(subdirs); do if make -C $$dir $@; then : else exit; fi; done 


