function v = helics_core_type_interprocess()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183036);
  end
  v = vInitialized;
end
