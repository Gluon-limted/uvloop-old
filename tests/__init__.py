import glob
import os
import sys
import unittest

if sys.platform in ('win32', 'cli'):
    exclude_tests = ('tests.test_tcp',)

    def load_tests(loader, tests, pattern):
        print(loader, tests, pattern)
        # top level directory cached on loader instance

        this_dir = os.path.dirname(__file__)
        suite = unittest.TestSuite()
        for test_class in loader.discover(start_dir=this_dir, pattern='test*.py'):

            for test in test_class :
                tests = []
                for unitTest in test._tests:
                    try:
                        print('###', unitTest.id())
                    except Exception as e:
                        print(repr(e))
                #import pdb;pdb.set_trace()
            continue
            #print('@@@@' + repr(test_class))
            if test_class not in exclude_tests:
                print(f'added {test_class}')
                tests = loader.loadTestsFromTestCase(test_class)
                suite.addTests(tests)
            else:
                print(f'excluded {test_class}')

        #sys.exit()
        return suite
