from pathlib import Path

def transform_snip(instance_name: str) -> None:

    """fix 1: We want to substitute 'obj' with 'Obj' in .sto
    fix 2: Coefs for sinkconst rows to be defined not only in .sto but in .cor
    Therefore, e.g., "node_851        sinkcons              1.0"
    should be present in .cor
    COLUMNS section in .cor should be ordered by columns, otherwise, duplicate name error.
    Because of that we cannot simply copy the lines.
    sinkcons line can differ for different scenarios.
    
    For this thing to work put .cor and .sto files to the cwd.
    """
    FIXED_PREFIX = ''

    input_cor = f'{instance_name}.cor'
    cor_file = Path(input_cor)
    if not cor_file.exists():
        raise FileNotFoundError(f'{input_cor} file was not found')
    cor_lines = cor_file.read_text().splitlines()
    cor_start = None
    cor_end = None
    for cor_line_index, cor_line in enumerate(cor_lines):
        if 'COLUMNS' in cor_line:
            cor_start = cor_line_index+1
        elif 'RHS' in cor_line:
            assert(cor_start is not None and 'We assume that RHS is the next section after COLUMNS')
            cor_end = cor_line_index
            break
    if cor_start is None:
        raise ValueError(f'COLUMNS section not found in {input_cor}')
    if cor_end is None:
        raise ValueError(f'RHS section not found in {input_cor}')

    input_sto = f'{instance_name}.sto'
    sto_file = Path(input_sto)
    if sto_file.exists():
        sto_contents = sto_file.read_text().replace('obj', 'Obj') # fix 1
        sto_lines = sto_contents.splitlines()
        sinkcons_lines = {}
        sinkcons_for_scen_found = False
        for sto_line_index, sto_line in enumerate(sto_lines):
            if 'sinkcons' in sto_line:
                assert (not sinkcons_for_scen_found and 'We assume that there is one sinkons line for every scenario')
                sto_column_name = sto_line.split()[0]
                sinkcons_lines[sto_column_name] = sto_line
                sinkcons_for_scen_found = True
            elif 'SC SCEN' in sto_line and 'SC SCEN1 ' not in sto_line:
                assert(sinkcons_for_scen_found and 'We assume that there is one sinkons line for every scenario')
                sinkcons_for_scen_found = False
        if not sinkcons_lines:
            raise ValueError(f'sinkons info not found in {input_sto}')

        # fix 2
        for sto_column_name, sto_line in sinkcons_lines.items():
            cor_line_index = cor_start
            while cor_line_index < cor_end:  # no guarantee that lines in .cor are ordered => restart for every sto_line
                cor_column_name = cor_lines[cor_line_index].split()[0]
                if cor_column_name == sto_column_name:
                    cor_lines.insert(cor_line_index, sto_line)
                    cor_end += 1  # inserted a line => COLUMNS ends one lines later
                    break
                else:
                    cor_line_index += 1
            else:
                raise ValueError(f'Data related to {sto_column_name} was not found in {input_cor} COLUMNS section')

        cor_lines_with_endl = [line + '\n' for line in cor_lines]
        output_cor = f'{FIXED_PREFIX}{input_cor}'
        with open(output_cor, 'w') as cor_file_fixed:
            cor_file_fixed.writelines(cor_lines_with_endl)
        print(f'Saved {output_cor}')

        sto_lines_with_endl = [line + '\n' for line in sto_lines]
        output_sto = f'{FIXED_PREFIX}{input_sto}'
        with open(output_sto, 'w') as sto_file_fixed:
            sto_file_fixed.writelines(sto_lines_with_endl)
        print(f'Saved {output_sto}')
    else:
        raise FileNotFoundError(f'{input_sto} file was not found')


def main():
    cwd = Path(__file__).cwd()
    instance_files = cwd.glob(f'snip*.sto')
    for instance in instance_files:
        transform_snip(instance.stem)


if __name__ == '__main__':
    main()