from __future__ import print_function

import argparse
import os
import os.path

from jinja2 import (ChoiceLoader, Environment,
                    PackageLoader, FileSystemLoader)
import ohaut_lib

OUTPUT_FILE = 'index.html'


def _get_arguments():
    parser = argparse.ArgumentParser(description="ohaut html render")
    parser.add_argument('input_file',  help='input html file')
    parser.add_argument('--set', '-set', nargs='+', dest='sets',
                        metavar="key=value")
    parser.add_argument('--template_path', '-t')
    parser.add_argument('output_dir',  help='output directory')
    parser.add_argument('--project_name', '-p', dest='project_name')

    args = parser.parse_args()

    kwargs = {'project_name': args.project_name}

    if args.sets:
        for key_val in args.sets:
            key, val = key_val.split('=')
            kwargs[key] = val

    return args, kwargs


def _write_output(output_dir, output_html):

    output_path = os.path.join(output_dir, OUTPUT_FILE)

    with open(output_path, 'w') as f:
        f.write(output_html)


def _copy_template_files(output_dir):
    templates_path = os.path.join(ohaut_lib.__path__[0], 'templates')
    os.system('cp -rf {} {}'.format(os.path.join(templates_path, '*'),
                                    output_dir))


def main():

    args, kwargs = _get_arguments()

    loaders = [FileSystemLoader(os.path.dirname(args.input_file))]

    if args.template_path:
        loaders.append(FileSystemLoader(args.template_path))
    else:
        loaders.append(PackageLoader('ohaut_lib', 'templates'))

    env = Environment(loader=ChoiceLoader(loaders), trim_blocks=True)

    template = env.get_template(os.path.basename(args.input_file))
    output_html = template.render(**kwargs)

    _write_output(args.output_dir, output_html)
    _copy_template_files(args.output_dir)
