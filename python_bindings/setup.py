import os
import sys
import setuptools
from glob import glob
from setuptools import setup, Extension
from setuptools.command.install import install
from distutils.command.build import build

__version__ = '1.0.0'

source_files = ['safran_wrapper.cpp', 'safran_wrapper.i']

source_files += glob('../src/*.cpp')
source_files = [src for src in source_files if src not in {'../src/SQLiteExplanation.cpp'}]

ext_modules = [
	Extension(
		'_safran_wrapper',
		source_files,
		swig_opts=["-c++", "-extranative"],
		language='c++',
		extra_compile_args=["-std=c++17", "-I../include", "-I../boost_1_76_0", "-fopenmp"],
		extra_link_args=["-fopenmp"],
	),
]

class CustomBuild(build):
	def run(self):
		self.run_command('build_ext')
		build.run(self)

class CustomInstall(install):
	def run(self):
		self.run_command('build_ext')
		self.do_egg_install()

custom_cmdclass = {'build': CustomBuild, 'install': CustomInstall}

setup(
	name='safran',
	version=__version__,
	description='SAFRAN: Scalable and fast non-redundant rule application',
	author='S. Ott, C. Melicke, M. Samwald, A. Belyy',
	url='https://github.com/AVBelyy/SAFRAN',
	ext_modules=ext_modules,
	cmdclass=custom_cmdclass,
	install_requires=[],
	setup_requires=[],
	py_modules=['safran', 'safran_wrapper'],
	zip_safe=False,
)
