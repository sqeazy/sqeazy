
((c++-mode . (
	      (eval . (set (make-local-variable 'my-project-path)
			   (file-name-directory
			    (let ((d (dir-locals-find-file ".")))
			      (if (stringp d) d (car d))))))
	      (eval . (progn
			(defun my-project-specific-function ()
			  
			  ;;;for auto-complete-clang-async
			  (if (boundp 'ac-clang-cflags)
			      (progn (setq ac-clang-cflags (append 
						      ac-clang-cflags '("-I/home/steinbac/development/sqeazy/inc" "-I/home/steinbac/development/sqeazy/src" "-I/home/steinbac/development/sqeazy/tests" "-I/home/steinbac/development/sqeazy/bench")
			  				)
					   )
				     (ac-clang-update-cmdlineargs)
				     )
			    nil)
			  
			  ;;;for company et al
			  (if (boundp 'company-clang-arguments)
			      (progn 
				(setq company-clang-arguments (append company-clang-arguments
								      (mapcar (lambda (item)(concat "-I" my-project-path item))
									      (split-string "inc src tests bench"))
								      '("-std=c++11")
								      )
				      )
				(message 
				 "[c++-mode] company-clang-arguments exists '%s'" company-clang-arguments)
				)
			    nil)

			  
			  )
			(my-project-specific-function)
			)
		    )
	      )
	   ))


;; ((company-mode . ((company-clang-arguments . ("-I/home/steinbac/development/sqeazy/inc"
;; 					      "-I/home/steinbac/development/sqeazy/src"
;; 					      "-I/home/steinbac/development/sqeazy/tests"
;; 					      "-I/home/steinbac/development/sqeazy/bench")))
;; 	       ))
