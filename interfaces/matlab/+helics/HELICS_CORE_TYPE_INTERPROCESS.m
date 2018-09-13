function v = HELICS_CORE_TYPE_INTERPROCESS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183095);
  end
  v = vInitialized;
end
