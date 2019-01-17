function v = helics_core_type_interprocess()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812626);
  end
  v = vInitialized;
end
