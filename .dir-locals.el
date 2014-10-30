((c++-mode . (
         (eval . (progn
                   (defun my-project-specific-function ()
		     (setq ac-clang-cflags (
					    append ac-clang-cflags '("-I/home/steinbac/development/sqeazy/inc" "-I/home/steinbac/development/sqeazy/src" "-I/home/steinbac/development/sqeazy/tests" "-I/home/steinbac/development/sqeazy/bench")
						   )
		     	   )
		     (ac-clang-update-cmdlineargs)
                     )
		   (my-project-specific-function)
		   )
	       )
	 )
))

((company-mode . ((company-clang-arguments . ("-I/home/<steinbac/development/sqeazy/inc"
					      "-I/home/<steinbac/development/sqeazy/src"
					      "-I/home/<steinbac/development/sqeazy/tests"
					      "-I/home/<steinbac/development/sqeazy/bench")))
	       ))
