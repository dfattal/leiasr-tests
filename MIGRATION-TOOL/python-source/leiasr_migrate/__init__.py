"""
LeiaSR Display API Migration Tool

Automated migration from legacy Display API to modern IDisplayManager with fallback.
"""

__version__ = "1.0.0"
__author__ = "Leia Inc."

from .analyzer import CodeAnalyzer, Detection
from .transformer import CodeTransformer, Transformation

__all__ = [
    'CodeAnalyzer',
    'Detection',
    'CodeTransformer',
    'Transformation',
]
