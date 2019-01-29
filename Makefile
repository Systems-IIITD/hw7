default: schedule

schedule: schedule.c
	gcc -o $@ $< 

clean:
	rm -rf schedule
