#!/usr/bin/perl
# -*- Mode: Perl; coding: utf-8; tab-width: 3; indent-tabs-mode: t; c-basic-offset: 3 -*-

package X3D::Parser;
use strict;
use warnings;
use v5.10.0;

use XML::LibXML;
#libxml-libxml-perl

use Glib;
#libglib-perl

my $DTDDirectory = "/usr/share/titania/dtd";
my $X3DCatalog   = "$DTDDirectory/x3d-catalog.xml";

my $RecommendedPublicId = "ISO//Web3D//DTD X3D 3.2//EN";
my $RecomemndedSystemId = "$DTDDirectory/x3d-3.2.dtd";

my $TitaniaInfo = "titania-info";

XML::LibXML -> load_catalog ($X3DCatalog);


sub new {
	my ($class, $options) = @_;

	my $document = new XML::LibXML ($options) -> load_xml ($options);

	my $self = {
		nodes                => new Glib::KeyFile (),
		dom                  => $document,
		specificationVersion => 3.3,
		profile              => undef,
		components           => [ ],
		metaData             => [ ],
		externProtos         => [ ],
		externProtosByName   => { },
		protos               => [ ],
		protosByName         => { },
		scene                => [ ],
		imports              => [ ],
		routes               => [ ],
		exports              => [ ],
		context              => [ ],
		parents              => [ ]
	};

	$self -> {nodes} -> load_from_data (join ("", `$TitaniaInfo -i 2>/dev/null`), "none");

	unshift @{$self -> {contexts}}, $self;

	bless $self, $class;
	return $self;
}

sub escape {
	my ($value) = @_;
	$value =~ s/(\A|[^\\])((?:\\\\)*["])/$1\\$2/sgo;  # Escape "
	return $value;
}

sub parse {
	my ($self) = @_;

	$self -> x3d ($self -> {dom} -> getDocumentElement ());
}

sub x3d {
	my ($self, $xml) = @_;

	$self -> {specificationVersion} = $xml -> getAttribute ("version");
	$self -> {profile}              = $xml -> getAttribute ("profile");

	$self -> validate ($xml);

	foreach my $child ($xml -> getChildnodes ())
	{
		if ($child -> nodeName () eq "head")
		{
			$self -> head ($child);
			next;
		}

		if ($child -> nodeName () eq "Scene")
		{
			$self -> scene ($child);
			next;
		}
	}
}

sub validate {
	my ($self, $xml) = @_;

	#say STDERR $self -> {dom} -> internalSubset () -> publicId ();
	#say STDERR $self -> {dom} -> internalSubset () -> systemId ();

	if (not $self -> {dom} -> internalSubset ())
	{
		my $publicId = "ISO//Web3D//DTD X3D " . $self -> {specificationVersion} . "//EN";
		my $systemId = "$DTDDirectory/x3d-" . $self -> {specificationVersion} . ".dtd";

		unless (-e $systemId)
		{
			$publicId = $RecommendedPublicId;
			$systemId = $RecomemndedSystemId;
		}

		$self -> {dom} -> createInternalSubset ($xml, $publicId, $systemId);

		eval { $self -> {dom} -> validate (); };
	}
}

sub head {
	my ($self, $xml) = @_;
	
	foreach my $child ($xml -> getChildnodes ())
	{
		if ($child -> nodeName () eq "component")
		{
			$self -> component ($child);
			next;
		}

		if ($child -> nodeName () eq "meta")
		{
			$self -> meta ($child);
			next;
		}
	}
}

sub component {
	my ($self, $xml) = @_;
	
	push @{$self -> {components}}, {
		name  => $xml -> getAttribute ("name"),
		level => $xml -> getAttribute ("level")
	};
}

sub meta {
	my ($self, $xml) = @_;
	
	push @{$self -> {metaData}}, {
		name  => $xml -> getAttribute ("name"),
		value => $xml -> getAttribute ("content")
	};
}

sub scene {
	my ($self, $xml) = @_;

	$self -> statements ($xml -> getChildnodes ());
}

sub statements {
	my $self = shift;
	
	foreach (@_)
	{
		$self -> statement ($_);
	}
}

sub statement {
	my ($self, $xml) = @_;

	if ($xml -> getName () eq "ExternProtoDeclare")
	{
		$self -> externProtoDeclare ($xml);
		return;
	}

	if ($xml -> getName () eq "ProtoDeclare")
	{
		$self -> protoDeclare ($xml);
		return;
	}

	if ($xml -> getName () eq "ProtoInstance")
	{
		$self -> protoInstance ($xml);
		return;
	}

	if ($xml -> getName () eq "IS")
	{
		$self -> is ($xml);
		return;
	}

	if ($xml -> getName () eq "ROUTE")
	{
		$self -> route ($xml);
		return;
	}

	if ($xml -> getName () eq "IMPORT")
	{
		$self -> import ($xml);
		return;
	}

	if ($xml -> getName () eq "EXPORT")
	{
		$self -> export ($xml);
		return;
	}

	$self -> node ($xml)
		if $xml -> nodeType () == XML_ELEMENT_NODE;
}

sub externProtoDeclare {
	my ($self, $xml) = @_;

	my $externProto = {
		name          => $xml -> getAttribute ("name"),
		appinfo       => $xml -> getAttribute ("appinfo"),
		documentation => $xml -> getAttribute ("documentation"),
		fields        => { },
		interface     => [ ],
		url           => $xml -> getAttribute ("url")
	};

	foreach my $child ($xml -> getChildnodes ())
	{
		if ($child -> nodeName () eq "field")
		{
			$self -> field ($child, $externProto, $externProto -> {interface});
			next;
		}
	}

	push @{$self -> {contexts} -> [0] -> {externProtos}}, $externProto;

	$self -> {contexts} -> [0] -> {externProtosByName} -> {$externProto -> {name}} = $externProto;
}

sub protoDeclare {
	my ($self, $xml) = @_;

	my $proto = {
		context            => $self -> {contexts} -> [0],
		name               => $xml -> getAttribute ("name"),
		appinfo            => $xml -> getAttribute ("appinfo"),
		documentation      => $xml -> getAttribute ("documentation"),
		fields             => { },
		interface          => [ ],
		externProtos       => [ ],
		externProtosByName => { },
		protos             => [ ],
		protosByName       => { },
		scene              => [ ],
		imports            => [ ],
		routes             => [ ],
		exports            => [ ]
	};

	foreach my $child ($xml -> getChildnodes ())
	{
		if ($child -> nodeName () eq "ProtoInterface")
		{
			$self -> protoInterface ($child, $proto, $proto -> {interface});
			next;
		}

		if ($child -> nodeName () eq "ProtoBody")
		{
			unshift @{$self -> {contexts}}, $proto;

			$self -> protoBody ($child);

			shift @{$self -> {contexts}};
			next;
		}
	}

	push @{$self -> {contexts} -> [0] -> {protos}}, $proto;
	$self -> {contexts} -> [0] -> {protosByName} -> {$proto -> {name}} = $proto;
}

sub protoInterface {
	my ($self, $xml, $proto, $interface) = @_;
	
	foreach my $child ($xml -> getChildnodes ())
	{
		if ($child -> nodeName () eq "field")
		{
			$self -> field ($child, $proto, $interface);
			next;
		}
	}
}

sub field {
	my ($self, $xml, $node, $interface) = @_;
	
	my $field = {
		userDefined   => 1,
		appinfo       => $xml -> getAttribute ("appinfo"),
		documentation => $xml -> getAttribute ("documentation"),
		accessType    => $xml -> getAttribute ("accessType"),
		type          => $xml -> getAttribute ("type"),
		name          => $xml -> getAttribute ("name")
	};

	$field -> {value} = $xml -> getAttribute ("value")
		if $xml -> getAttribute ("value");

	$node -> {fields} -> {$field -> {name}} = $field;
	push @$interface, $field;

	unshift @{$self -> {parents}}, $field;
	$self -> statements ($xml -> getChildnodes ());
	shift @{$self -> {parents}};
}

sub protoBody {
	my ($self, $xml) = @_;
	
	$self -> statements ($xml -> getChildnodes ());
}

sub protoInstance {
	my ($self, $xml) = @_;

	my $node = {
		userDefined       => 1,
		context           => $self -> {contexts} -> [0],
	   typeName          => $xml -> getAttribute ("name"),
		fields            => { },
		userDefinedFields => [ ]
	};

	if ($xml -> getAttribute ("USE"))
	{
		$node -> {reference} = $xml -> getAttribute ("USE");
	}
	else
	{
		$node -> {name} = $xml -> getAttribute ("DEF");

		unshift @{$self -> {parents}}, $node;
	
		foreach my $child ($xml -> getChildnodes ())
		{
			if ($child -> nodeName () eq "fieldValue")
			{
				$self -> fieldValue ($child, $node -> {fields});
				next;
			}
		}

		shift @{$self -> {parents}};
	}

	$self -> addNode ($xml, $node);
}

sub fieldValue {
	my ($self, $xml, $fields) = @_;
	
	my $name = $xml -> getAttribute ("name");

	my $field = {
		fieldValue => 1,
		name       => $name
	};

	$fields -> {$name} = $field;

	$field -> {value} = $xml -> getAttribute ("value")
		if $xml -> getAttribute ("value");

	unshift @{$self -> {parents}}, $field;
	$self -> statements ($xml -> getChildnodes ());
	shift @{$self -> {parents}};
}

sub is {
	my ($self, $xml) = @_;
	
	my $node              = $self -> {parents} -> [0];
	my $fields            = $node -> {fields};
	
	foreach ($xml -> getChildnodes ())
	{
		$self -> connect ($_, $node, $fields)
			if $_ -> getName () eq "connect";
	}
}

sub connect {
	my ($self, $xml, $node, $fields) = @_;

	my $nodeField = $xml -> getAttribute ("nodeField");

	$fields -> {$nodeField} = { }
		unless exists $fields -> {$nodeField};
		
	my $field = $fields -> {$nodeField};

	$field -> {name}      = $nodeField;
	$field -> {reference} = $xml -> getAttribute ("protoField");
}

### save container field
open CONTAINER_FIELD, ">/home/holger/Projekte/Titania/Papers/ContainerField.txt";

sub node {
	my ($self, $xml) = @_;
	
	my $node = {
		context           => $self -> {contexts} -> [0],
	   typeName          => $xml -> getName (),
		fields            => { },
		userDefinedFields => [ ]
	};

	if ($xml -> getAttribute ("USE"))
	{
		$node -> {reference} = $xml -> getAttribute ("USE");
	}
	else
	{
		$node -> {name} = $xml -> getAttribute ("DEF");

		unshift @{$self -> {parents}}, $node;

		$self -> attributes ($xml, $node -> {fields});
	
		foreach my $child ($xml -> getChildnodes ())
		{
			if ($child -> getName () eq "field")
			{
				$self -> field ($child, $node, $node -> {userDefinedFields});
				next;
			}
			
			if ($child -> getName () eq "#cdata-section")
			{
				if ($self -> {nodes} -> has_key ($node -> {typeName}, "url"))
				{
					$self -> cdata ($child, $node -> {fields});
					next;
				}
			}

			$self -> statement ($child);
		}

		shift @{$self -> {parents}};
	}

	$self -> addNode ($xml, $node);

	### save container field

	say CONTAINER_FIELD "[", $xml -> getName (), "]";
	say CONTAINER_FIELD "\tcontainerField = ", $xml -> getAttribute ("containerField");
	say CONTAINER_FIELD "";

	###
}

sub cdata {
	my ($self, $xml, $fields) = @_;
	
	if ($fields -> {url})
	{
		if ($fields -> {url} -> {value})
		{
			if ($fields -> {url} -> {value} !~ /^\s*"/sgo)
			{
				$fields -> {url} -> {value} = "\"" . $fields -> {url} -> {value} . "\""
			}
		}
		else
		{
			$fields -> {url} -> {value} = "";
		}
	}
	else
	{
		$fields -> {url} = {
			name  => "url",
			value => ""
		};
	}

	my $value = $xml -> getValue ();

	$value =~ s/\A\s*//sgo;      # Remove spaces from beginning
	$value =~ s/\s*$//sgo;       # Remove spaces from end

	$fields -> {url} -> {value} .= " \"" . escape ($value) . "\"";
}

sub addNode {
	my ($self, $xml, $node) = @_;
	
	if (@{$self -> {parents}})
	{
		my $parent = $self -> {parents} -> [0];
		my $field  = $parent;
	
		if (exists $parent -> {fields})
		{
			# Parent is a node
		
			my $fields         = $parent -> {fields};
			my $containerField = $xml -> getAttribute ("containerField");
			
			$fields -> {$containerField} = {
				name  => $containerField,
				value => [ ]
			}
			unless exists $fields -> {$containerField};
		
			$field = $fields -> {$containerField};
		}
		# else parent is a custom field

		$field -> {value} = [ ]
			unless exists $field -> {value};

		push @{$field -> {value}}, $node;
	}
	else
	{
		push @{$self -> {contexts} -> [0] -> {scene}}, $node;
	}
}

sub attributes {
	my ($self, $xml, $fields) = @_;
	
	foreach my $attribute ($xml -> getAttributes ())
	{
		my $name = $attribute -> getName ();

		next if $name =~ /DEF|USE|containerField|class/sgo;

		$fields -> {$name} = {
			name  => $name,
			value => $attribute -> getValue ()
		};
	}
}

sub import {
	my ($self, $xml) = @_;

	push @{$self -> {contexts} -> [0] -> {imports}}, {
		inline       => $xml -> getAttribute ("inlineDEF"),
		exportedName => $xml -> getAttribute ("exportedDEF"),
		localName    => $xml -> getAttribute ("AS")
	};
}

sub route {
	my ($self, $xml) = @_;

	push @{$self -> {contexts} -> [0] -> {routes}}, {
		fromNode  => $xml -> getAttribute ("fromNode"),
		fromField => $xml -> getAttribute ("fromField"),
		toNode    => $xml -> getAttribute ("toNode"),
		toField   => $xml -> getAttribute ("toField")
	};
}

sub export {
	my ($self, $xml) = @_;

	push @{$self -> {contexts} -> [0] -> {exports}}, {
		localName    => $xml -> getAttribute ("localDEF"),
		exportedName => $xml -> getAttribute ("AS")
	};
}

1;
package X3D::Generator;
use strict;
use warnings;
use v5.10.0;

use Glib;
#libglib-perl

binmode STDOUT, ":utf8";

sub new {
	my ($class, $document) = @_;

	my $self = {
		document => $document,
		nodes    => $document -> {nodes},
		fields   => new Glib::KeyFile (),
		indent   => 0
	};

	$self -> {fields} -> load_from_data (join ("", `$TitaniaInfo -f 2>/dev/null`), "none");

	bless $self, $class;
	return $self;
}

sub escape {
	return X3D::Parser::escape (@_);
}

sub getPrototype {
	my ($self, $context, $name) = @_;

	return $context -> {externProtosByName} -> {$name}
		if exists $context -> {externProtosByName} -> {$name};

	return $context -> {protosByName} -> {$name}
		if exists $context -> {protosByName} -> {$name};

	return $self -> getPrototype ($context -> {context}, $name)
		if $context -> {context};
}

sub output {
	my ($self) = @_;
	$self -> scene ();
}

sub scene {
	my ($self) = @_;
	
	print "#X3D V", $self -> {document} -> {specificationVersion}, " utf8\n\n";
	
	print "PROFILE ", $self -> {document} -> {profile}, "\n\n"
		if $self -> {document} -> {profile};

	$self -> components ($self -> {document} -> {components});
	$self -> metaData ($self -> {document} -> {metaData});

	$self -> externProtos (@{$self -> {document} -> {externProtos}});
	$self -> protos       (@{$self -> {document} -> {protos}});
	$self -> nodes        (@{$self -> {document} -> {scene}});
	$self -> imports      (@{$self -> {document} -> {imports}});
	$self -> routes       (@{$self -> {document} -> {routes}});
	$self -> exports      (@{$self -> {document} -> {exports}});
}

sub components {
	my ($self, $components) = @_;
	
	foreach (@$components)
	{
		print "COMPONENT ", $_ -> {name}, " : ", $_ -> {level}, "\n";
	}

	print "\n" if @$components;
}

sub metaData {
	my ($self, $metaData) = @_;
	
	foreach (@$metaData)
	{
		print "META \"", escape ($_ -> {name}), "\" \"", escape ($_ -> {value}), "\"\n";
	}
	
	print "\n" if @$metaData;
}

sub externProtos {
	my $self = shift;
	
	for (@_)
	{
		print "  " x $self -> {indent};
		$self -> externProto ($_);
		print "\n\n";
	}

	print "\n" if @_;
}

sub externProto {
	my ($self, $externProto) = @_;

	print "EXTERNPROTO ", $externProto -> {name}, " [";
	
	if (@{$externProto -> {interface}})
	{
		print "\n";
		++ $self -> {indent};
	
		$self -> restrictedInterfaceDeclarations (@{$externProto -> {interface}});
	
		-- $self -> {indent};
		print "  " x $self -> {indent};
		print "]\n";
	}
	else
	{
		print " ]\n";
	}

	print "  " x $self -> {indent};
	$self -> MFString ($externProto -> {url});
}

sub restrictedInterfaceDeclarations {
	my $self = shift;

	foreach (@_)
	{
		$self -> restrictedInterfaceDeclaration ($_);
	}
}

sub restrictedInterfaceDeclaration {
	my ($self, $field) = @_;

	$self -> documentation ($field);

	print "  " x $self -> {indent};
	print
		$field -> {accessType},
		" ",
		$field -> {type},
		" ",
		$field -> {name},
		"\n";
}

sub documentation {
	my ($self, $object) = @_;
	
	print "  " x $self -> {indent}, "# " , $object -> {appinfo}, "\n"
		if $object -> {appinfo};
		
	if ($object -> {documentation})
	{
		print "  " x $self -> {indent}, "# \n"
			if $object -> {appinfo};

		print "  " x $self -> {indent}, "# DOCUMENTATION\n";
		print "  " x $self -> {indent}, "# " , $object -> {documentation}, "\n";
	}
}

sub protos {
	my $self = shift;

	for (@_)
	{
		print "  " x $self -> {indent};
		$self -> proto ($_);
		print "\n\n";
	}

	print "\n" if @_;
}

sub proto {
	my ($self, $proto) = @_;

	$self -> documentation ($proto);

	print "PROTO ", $proto -> {name}, " [";

	if (@{$proto -> {interface}})
	{
		print "\n";
		++ $self -> {indent};
	
		$self -> interfaceDeclarations (@{$proto -> {interface}});
	
		-- $self -> {indent};
		print "  " x $self -> {indent};
		print "]\n";
	}
	else
	{
		print " ]\n";
	}
	
	print "  " x $self -> {indent};
	print "{\n";
	++ $self -> {indent};

	$self -> externProtos (@{$proto -> {externProtos}});
	$self -> protos       (@{$proto -> {protos}});
	$self -> nodes        (@{$proto -> {scene}});
	$self -> imports      (@{$proto -> {imports}});
	$self -> routes       (@{$proto -> {routes}});

	-- $self -> {indent};
	print "  " x $self -> {indent};
	print "}\n";
}

sub interfaceDeclarations {
	my $self = shift;

	foreach (@_)
	{
		$self -> interfaceDeclaration ($_);
	}
}

sub interfaceDeclaration {
	my ($self, $field) = @_;

	$self -> documentation ($field);

	print "  " x $self -> {indent};
	print
		$field -> {accessType},
		" ",
		$field -> {type},
		" ",
		$field -> {name};
	
	if ($field -> {accessType} =~ /initializeOnly|inputOutput/sgo)
	{
		print " ";
		$self -> field ([$field -> {name}, $field -> {accessType}, $field -> {type}], $field);
	}

	print "\n";
}

sub nodes {
	my $self = shift;
	
	for (@_)
	{
		print "  " x $self -> {indent};
		$self -> node ($_);
		print "\n\n";
	}

	print "\n" if @_;
}

sub node {
	my ($self, $node) = @_;
	
	if ($node -> {reference})
	{
		print "USE ", $node -> {reference};
		return;
	}

	if ($self -> {nodes} -> has_group ($node -> {typeName}) or $node -> {userDefined})
	{
		print "DEF ", $node -> {name}, " " if $node -> {name};
		print $node -> {typeName}, " {";

		if (%{$node -> {fields}})
		{
			print "\n";
			++ $self -> {indent};
		
			$self -> fields ($node);
			$self -> userDefinedFields ($node);

			-- $self -> {indent};
			print "  " x $self -> {indent};
			print "}";		
		}
		else
		{
			print " }";
		}
	}
}

sub fields {
	my ($self, $node) = @_;

	foreach (values %{$node -> {fields}})
	{
		next if $_ -> {userDefined};
	
		print "  " x $self -> {indent};
		print $_ -> {name}, " ";

		if ($_ -> {reference})
		{
			print "IS ", $_ -> {reference};
		}
		else
		{
			if ($_ -> {fieldValue})
			{
				my $proto           = $self -> getPrototype ($node -> {context}, $node -> {typeName});
				my $fieldDefinition = $proto -> {fields} -> {$_ -> {name}};
				
				$self -> field ([$fieldDefinition -> {name}, $fieldDefinition -> {accessType}, $fieldDefinition -> {type}], $_);	
			}
			else
			{
				if ($self -> {nodes} -> has_key ($node -> {typeName}, $_ -> {name}))
				{
					$self -> field ([$self -> {nodes} -> get_string_list ($node -> {typeName}, $_ -> {name})], $_);
				}
			}
		}
						
		print "\n";
	}
}

sub userDefinedFields {
	my ($self, $node) = @_;

	foreach (@{$node -> {userDefinedFields}})
	{
		print "  " x $self -> {indent};
		print $_ -> {accessType}, " ";
		print $_ -> {type}, " ";
		print $_ -> {name};

		if ($_ -> {reference})
		{
			print " IS ", $_ -> {reference};
		}
		else
		{
			if ($_ -> {accessType} =~ /initializeOnly|inputOutput/sgo)
			{
				print " ";
				$self -> field ([$_ -> {name}, $_ -> {accessType}, $_ -> {type}], $_);	
			}
		}
						
		print "\n";
	}
}

sub field {
	my ($self, $fieldDefinition, $field) = @_;

	my $typeName = $fieldDefinition -> [2];

	if ($typeName eq "SFBool")
	{
		$field -> {value}
		? print uc $field -> {value}
		: print "FALSE";
	}
	elsif ($typeName eq "SFNode")
	{
		$field -> {value}
		? $self -> node ($field -> {value} -> [0])
		: print "NULL";
	}
	elsif ($typeName eq "SFString")
	{
		$field -> {value}
		? print "\"", escape ($field -> {value}), "\""
		: print "\"\"";
	}
	elsif ($typeName eq "MFBool")
	{
		$field -> {value}
		? print "[ ", uc ($field -> {value}) , "]"
		: print "[ ]";
	}
	elsif ($typeName eq "MFNode")
	{
		if ($field -> {value})
		{
			if (ref ($field -> {value}) eq ref ([ ]))
			{
				print "[\n";
				++ $self -> {indent};
		
				foreach (@{$field -> {value}})
				{
					print "  " x $self -> {indent};

					$self -> node ($_);
					print "\n";
				}

				-- $self -> {indent};
				print "  " x $self -> {indent};
				print "]";
			}
			else
			{
				print "[ ", uc ($field -> {value}), " ]";
			}
		}
		else
		{
			print "[ ]";
		}
	}
	elsif ($typeName eq "MFString")
	{
		$self -> MFString ($field -> {value});
	}
	elsif ($typeName =~ /^SF/sgo)
	{
		if ($field -> {value})
		{
			print $field -> {value};
		}
		else
		{
			print $self -> {fields} -> get_string ($typeName, "value")
				if $self -> {fields} -> has_group ($typeName);
		}
	}
	else
	{
		$field -> {value}
		? print "[ ", $field -> {value} , " ]"
		: print "[ ]";
	}
}

sub MFString {
	my ($self, $value) = @_;

	if ($value)
	{
		if ($value =~ /^\s*"/sgo)
		{
			print "[ ", $value , "]";
		}
		else
		{
			print "[ \"", escape ($value) , "\" ]";
		}
	}
	else
	{
		print "[ ]";
	}
}

sub imports {
	my $self = shift;

	for (@_)
	{
		print "  " x $self -> {indent};
		$self -> import ($_);
		print "\n";
	}

	print "\n" if @_;
}

sub import {
	my ($self, $import) = @_;
	
	print "IMPORT ";
	print $import -> {inline}, ".", $import -> {exportedName};
	
	if ($import -> {localName})
	{
		print " AS ";
		print $import -> {localName};
	}
}

sub routes {
	my $self = shift;

	for (@_)
	{
		print "  " x $self -> {indent};
		$self -> route ($_);
		print "\n";
	}

	print "\n" if @_;
}

sub route {
	my ($self, $route) = @_;
	
	print "ROUTE ";
	print $route -> {fromNode}, ".", $route -> {fromField};
	print " TO ";
	print $route -> {toNode}, ".", $route -> {toField};
}

sub exports {
	my $self = shift;

	for (@_)
	{
		print "  " x $self -> {indent};
		$self -> export ($_);
		print "\n";
	}

	print "\n" if @_;
}

sub export {
	my ($self, $export) = @_;

	print "EXPORT ";
	print $export -> {localName};

	if ($export -> {exportedName})
	{
		print " AS ";
		print $export -> {exportedName};
	}
}

1;
package main;
use strict;
use warnings;
use v5.10.0;

my $options = {
	recover  => 2,
	location => "/home/holger/Projekte/Titania/Library/Tests/Basic/NodeIndex.x3d"
};

my $parser = new X3D::Parser ($options);
$parser -> parse ();

my $generator = new X3D::Generator ($parser);
$generator -> output ();

#print STDERR $@ if $@;
