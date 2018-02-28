function v = HELICS_CORE_TYPE_TEST()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 36);
  end
  v = vInitialized;
end
