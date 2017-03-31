
function isnum(n) {
	return n ~ /^[+-]?[0-9]+$/
}

{
	for (i = 1; i <= NF; i++)
		if (isnum($i))
			sum[i] += $i
	if (NF > maxfld)
		maxfld = NF
}

END {
	for (i = 1; i < maxfld; i++)
		printf("%d\t", sum[i]);
	printf("%d\n", sum[maxfld]);
}
