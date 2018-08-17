function v = helics_clone_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783867);
  end
  v = vInitialized;
end
