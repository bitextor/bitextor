from setuptools import setup
import sys, os

version = '0.9'

setup(name='ulyses',
      version=version,
      description="ulyses.py is a standalone sentences splitter.",
      long_description="""\
""",
      packages = ['ulyses'],
      classifiers=[], # Get strings from http://pypi.python.org/pypi?%3Aaction=list_classifiers
      keywords='sentence splitting',
      author='Sergio Ortiz Rojas',
      author_email='sergio@prompsit.com',
      url='https://github.com/sortiz/ulysses-sentence-splitter',
      license='GPLv2',
      zip_safe=False,
      py_modules=['ulyses'],
      install_requires=[
          # -*- Extra requirements: -*-
          'regex',
      ],
#      scripts = ['ulyses.py'],
      entry_points= {
        'console_scripts': [
          'ulyses = ulyses.ulyses:main',
        ],
      },
      )
