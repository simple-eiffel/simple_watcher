note
	description: "Test application for simple_watcher"
	date: "$Date$"
	revision: "$Revision$"

class
	TEST_APP

create
	make

feature -- Initialization

	make
			-- Run every test once: a failing test is counted and reported, and the run goes on to
			-- the next (it used to restart the whole run from the first test, without end).
		do
			print ("Testing SIMPLE_WATCHER...%N%N")
			create tests
			run_test (agent tests.test_watcher_creation, "test_watcher_creation")
			run_test (agent tests.test_watcher_start, "test_watcher_start")
			run_test (agent tests.test_created_file_is_reported, "test_created_file_is_reported")
			run_test (agent tests.test_subfolder_file_is_reported, "test_subfolder_file_is_reported")
			run_test (agent tests.test_new_folder_is_watched, "test_new_folder_is_watched")
			run_test (agent tests.test_subfolder_is_silent_without_subtree, "test_subfolder_is_silent_without_subtree")
			run_test (agent tests.test_watcher_close, "test_watcher_close")
			run_test (agent tests.test_watch_path, "test_watch_path")
			run_test (agent tests.test_poll_no_changes, "test_poll_no_changes")
			run_test (agent tests.test_invalid_directory, "test_invalid_directory")
			run_test (agent tests.test_watch_flag_constants, "test_watch_flag_constants")
			run_test (agent tests.test_event_type_constants, "test_event_type_constants")
			run_test (agent tests.test_watch_event_class, "test_watch_event_class")
			print ("%N======================================%N")
			print ("Results: " + passed.out + " passed, " + failed.out + " failed%N")
		end

feature {NONE} -- Running

	tests: LIB_TESTS
			-- The tests, with their fixture.

	passed, failed: INTEGER
			-- How many tests passed, and failed.

	run_test (a_test: PROCEDURE; a_name: STRING)
			-- Prepare, run `a_test' and clean up; a failure is counted, its reason printed, and the
			-- fixture still cleaned.
		local
			l_failed: BOOLEAN
		do
			print ("  " + a_name + ": ")
			if not l_failed then
				tests.on_prepare
				a_test.call (Void)
				tests.on_clean
				print ("PASSED%N")
				passed := passed + 1
			else
				print ("FAILED%N")
				if attached (create {EXCEPTION_MANAGER}).last_exception as l_e and then attached l_e.description as l_why then
					print ("        " + {UTF_CONVERTER}.string_32_to_utf_8_string_8 (l_why.to_string_32) + "%N")
				end
				failed := failed + 1
				clean_after_failure
			end
		rescue
			l_failed := True
			retry
		end

	clean_after_failure
			-- `on_clean', whose own failure must not stop the run.
		local
			l_tried: BOOLEAN
		do
			if not l_tried then
				tests.on_clean
			end
		rescue
			l_tried := True
			retry
		end

end
