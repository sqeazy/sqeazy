
((c++-mode . (
	      (eval . (set (make-local-variable 'my-project-path)
			   (file-name-directory
			    (let ((d (dir-locals-find-file ".")))
			      (if (stringp d) d (car d))))))
	      (eval . (progn
			(defun my-project-specific-function ()

			  ;;;for auto-complete-clang-async
			  (if (boundp 'ac-clang-cflags)
				  (progn (setq ac-clang-cflags
					   (append ac-clang-cflags
                                      (mapcar (lambda (item)(concat "-I" my-project-path "/src/cpp/" item))
									      (split-string "inc src tests bench"))
								      '("-std=c++11")
								      )
					   )
				     (ac-clang-update-cmdlineargs)
				     (message "[c++-mode] ac-clang-cflags updated")

				     )
			    nil)

			  ;;for auto-complete-clang
			  (if (boundp 'ac-clang-flags)
			      (progn (setq ac-clang-flags 
					   (append ac-clang-flags
                               (mapcar (lambda (item)(concat "-I" my-project-path "/src/cpp/" item))
									      (split-string "inc src tests bench"))
								      '("-std=c++11")
								      )
					   )
				     ;;(ac-clang-update-cmdlineargs)
				     (message "[c++-mode] ac-clang-flags updated")

				     )
			    nil)

			  ;;;for company et al
			  (if (boundp 'company-clang-arguments)
			      (progn 
				(setq company-clang-arguments (append company-clang-arguments
                                                      (mapcar (lambda (item)(concat "-I" my-project-path "/src/cpp/" item))
									      (split-string "inc src tests bench"))
								      '("-std=c++11")
								      )
                      )
                (add-to-list 'company-c-headers-path-system (concat my-project-path "/src/cpp/src" ))
                (add-to-list 'company-c-headers-path-system (concat my-project-path "/src/cpp/inc" ))
                (add-to-list 'company-c-headers-path-system (concat my-project-path "/src/cpp/tests" ))

				(message "[c++-mode] company-clang-arguments updated" )
				)
			    nil)

			  ;;;for semantic-mode
;;;              (if (boundp 'global-semanticdb-minor-mode)
;;;                  (progn
;;;                    (semantic-add-system-include (concat my-project-path "/src/cpp/src" ))
;;;                    (semantic-add-system-include (concat my-project-path "/src/cpp/inc" ))
;;;                    (semantic-add-system-include (concat my-project-path "/src/cpp/tests" ))
;;;
;;;                    (message "[c++-mode] semantic-add-system-include updated" )
;;;                    )
;;;                nil)
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
