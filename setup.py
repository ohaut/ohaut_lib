from setuptools import setup, find_packages
from codecs import open
from os import path, walk
from ohaut_tools import __version__

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='utf-8') as readme_f:
    long_description = readme_f.read()

remove = data_dir = path.join(here)
len_remove = len(remove)+1

data_dir = path.join(here, 'ohaut_tools', 'templates')
data_files = [(d[len_remove:], [path.join(d[len_remove:],f) for f in files])
              for d, folders, files in walk(data_dir)]

setup(
    name='ohaut-lib',

    version=__version__,
    description='OHAUT Library, HTML and tools',
    long_description=long_description,

    # The project's main homepage.
    url='https://github.com/ohaut/ohaut-lib',

    # Author details
    author='Miguel Angel Ajo',
    author_email='miguelangel@ajo.es',

    # Choose your license
    license='GPL3',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 3 - Alpha',

        # Indicate who your project is intended for
        'Intended Audience :: Developers',

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: GPL3',
        'Topic :: Utilities',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 3',
    ],

    # What does your project relate to?
    keywords='Open Home Automation OHAUT',

    # You can just specify the packages manually here if your project is
    # simple. Or you can use find_packages().
    packages=find_packages(exclude=['contrib', 'docs', 'tests']),

    py_modules=['ohaut_tools'],

    install_requires=['jinja2'],
    # extras_require={
    #    'dev': ['check-manifest'],
    #    'test': ['coverage'],
    # },
    data_files=['README.rst'] + data_files,

    entry_points={
        'console_scripts': [
            'ohaut-render-html=ohaut_tools.render_html:main',
        ],
    },
)

