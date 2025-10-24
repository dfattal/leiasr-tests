#!/usr/bin/env python3
"""
Setup script for LeiaSR Display API Migration Tool
"""

from setuptools import setup, find_packages
from pathlib import Path

# Read README
readme_file = Path(__file__).parent / 'README.md'
long_description = readme_file.read_text() if readme_file.exists() else ''

setup(
    name='leiasr-migrate',
    version='1.0.0',
    description='Automated migration tool for LeiaSR Display API to modern IDisplayManager',
    long_description=long_description,
    long_description_content_type='text/markdown',
    author='Leia Inc.',
    python_requires='>=3.7',
    packages=find_packages(),
    include_package_data=True,
    package_data={
        'leiasr_migrate': ['patterns/*.yaml'],
    },
    install_requires=[
        'pyyaml>=6.0',
    ],
    entry_points={
        'console_scripts': [
            'leiasr-migrate=leiasr_migrate.cli:main',
        ],
    },
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
    ],
)
