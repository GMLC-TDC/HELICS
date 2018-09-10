function v = helics_custom_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535383);
  end
  v = vInitialized;
end
