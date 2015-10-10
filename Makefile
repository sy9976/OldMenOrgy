MPICC  = mpicc

FLAGS   =  -lm

TARGETS = orgy

all : $(TARGETS)
	@echo Make Complete
	
orgy : orgy.c parameters.c parameters.h semaphores.c semaphores.h communication.c communication.h
	$(MPICC) orgy.c parameters.c semaphores.c communication.c $(FLAGS) -o orgy

